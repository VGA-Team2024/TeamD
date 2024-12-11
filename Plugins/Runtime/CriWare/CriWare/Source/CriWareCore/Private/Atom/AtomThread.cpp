/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCore
 * File     : AtomThread.cpp
 *
 ****************************************************************************/

#include "Atom/AtomThread.h"

#include "HAL/PlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "HAL/ExceptionHandling.h"
#include "Misc/CoreStats.h"
#include "UObject/UObjectGlobals.h"
#include "HAL/LowLevelMemTracker.h"
#include "Async/Async.h"
#include "Tasks/Pipe.h"

#include "CriWareLLM.h"
#include "CriWareCore.h"
#include "Atom/Atom.h"

//
// Globals
//

extern CRIWARECORE_API UE::Tasks::FPipe GAtomPipe;
extern CRIWARECORE_API std::atomic<bool> GIsAtomThreadRunning;
extern CRIWARECORE_API std::atomic<bool> GIsAtomThreadSuspended;

static int32 GCVarSuspendAtomThread = 0;
FAutoConsoleVariableRef CVarSuspendAtomThread(TEXT("AtomThread.SuspendAtomThread"), GCVarSuspendAtomThread, TEXT("0=Resume, 1=Suspend"), ECVF_Cheat);

static int32 GCVarAboveNormalAtomThreadPri = 0;
FAutoConsoleVariableRef CVarAboveNormalAtomThreadPri(TEXT("AtomThread.AboveNormalPriority"), GCVarAboveNormalAtomThreadPri, TEXT("0=Normal, 1=AboveNormal"), ECVF_Default);

static int32 GCVarEnableAtomCommandLogging = 0;
FAutoConsoleVariableRef CVarEnableAtomCommandLogging(TEXT("AtomThread.EnableAtomCommandLogging"), GCVarEnableAtomCommandLogging, TEXT("0=Disabled, 1=Enabled"), ECVF_Default);

static int32 GCVarEnableAtomBatchProcessing = 1;
FAutoConsoleVariableRef CVarEnableAtomBatchProcessing(
	TEXT("AtomThread.EnableBatchProcessing"),
	GCVarEnableAtomBatchProcessing,
	TEXT("Enables batch processing atom thread commands.\n")
	TEXT("0: Not Enabled, 1: Enabled"),
	ECVF_Default);

static int32 GBatchAtomAsyncBatchSize = 128;
static FAutoConsoleVariableRef CVarBatchAtomAsyncBatchSize(
	TEXT("AtomThread.BatchAsyncBatchSize"),
	GBatchAtomAsyncBatchSize,
	TEXT("When AtomThread.EnableBatchProcessing = 1, controls the number of atom commands grouped together for threading.")
);

static int32 GAtomCommandFenceWaitTimeMs = 35;
FAutoConsoleVariableRef  CVarAtomCommandFenceWaitTimeMs(
	TEXT("AtomCommand.FenceWaitTimeMs"),
	GAtomCommandFenceWaitTimeMs,
	TEXT("Sets number of ms for fence wait"),
	ECVF_Default);

struct FAtomThreadInteractor
{
	static void UseAtomThreadCVarSinkFunction()
	{
		static bool bLastSuspendAtomThread = false;
		const bool bSuspendAtomThread = GCVarSuspendAtomThread != 0;

		if (bLastSuspendAtomThread != bSuspendAtomThread)
		{
			bLastSuspendAtomThread = bSuspendAtomThread;
			if (bSuspendAtomThread && IsAtomThreadRunning())
			{
				FAtomThread::SuspendAtomThread();
			}
			else if (GIsAtomThreadSuspended)
			{
				FAtomThread::ResumeAtomThread();
			}
			else if (GIsEditor)
			{
				UE_LOG(LogCriWareAtom, Warning, TEXT("Atom threading is disabled in the editor."));
			}
			else if (!FAtomThread::IsUsingThreadedAtom())
			{
				UE_LOG(LogCriWareAtom, Warning, TEXT("Cannot manipulate atom thread when disabled by platform or ini."));
			}
		}
	}
};

UE::Tasks::ETaskPriority GAtomTaskPriority = UE::Tasks::ETaskPriority::Normal;

static void SetAtomTaskPriority(const TArray<FString>& Args)
{
	UE_LOG(LogConsoleResponse, Display, TEXT("AtomTaskPriority was %s."), LowLevelTasks::ToString(GAtomTaskPriority));

	if (Args.Num() > 1)
	{
		UE_LOG(LogConsoleResponse, Display, TEXT("WARNING: This command requires a single argument while %d were provided, all extra arguments will be ignored."), Args.Num());
	}
	else if (Args.IsEmpty())
	{
		UE_LOG(LogConsoleResponse, Display, TEXT("ERROR: Please provide a new priority value."));
		return;
	}

	if (!LowLevelTasks::ToTaskPriority(*Args[0], GAtomTaskPriority))
	{
		UE_LOG(LogConsoleResponse, Display, TEXT("ERROR: Invalid priority: %s."), *Args[0]);
	}

	UE_LOG(LogConsoleResponse, Display, TEXT("Atom Task Priority was set to %s."), LowLevelTasks::ToString(GAtomTaskPriority));
}

static FAutoConsoleCommand AtomThreadPriorityConsoleCommand(
	TEXT("AtomThread.TaskPriority"),
	TEXT("Takes a single parameter of value `High`, `Normal`, `BackgroundHigh`, `BackgroundNormal` or `BackgroundLow`."),
	FConsoleCommandWithArgsDelegate::CreateStatic(&SetAtomTaskPriority)
);

static FAutoConsoleVariableSink CVarUseAtomThreadSink(FConsoleCommandDelegate::CreateStatic(&FAtomThreadInteractor::UseAtomThreadCVarSinkFunction));

bool FAtomThread::bUseThreadedAtom = false;

FCriticalSection FAtomThread::CurrentAtomThreadStatIdCS;
TStatId FAtomThread::CurrentAtomThreadStatId;
TStatId FAtomThread::LongestAtomThreadStatId;
double FAtomThread::LongestAtomThreadTimeMsec = 0.0;

#if UE_ATOM_THREAD_AS_PIPE

TUniquePtr<UE::Tasks::FTaskEvent> FAtomThread::ResumeEvent;
int32 FAtomThread::SuspendCount{ 0 };

void FAtomThread::SuspendAtomThread()
{
	check(IsInGameThread()); // thread-safe version would be much more complicated

	if (!GIsAtomThreadRunning)
	{
		return; // nothing to suspend
	}

	if (++SuspendCount != 1)
	{
		return; // recursive scope
	}

	check(!GIsAtomThreadSuspended.load(std::memory_order_relaxed));

	FEventRef SuspendEvent;

	FAtomThread::RunCommandOnAtomThread(
		[&SuspendEvent]
		{
			GIsAtomThreadSuspended.store(true, std::memory_order_release);
			SuspendEvent->Trigger();

			ResumeEvent->BusyWait(); // don't block one of workers while atom pipe is suspended
			ResumeEvent = MakeUnique<UE::Tasks::FTaskEvent>(UE_SOURCE_LOCATION); // thread-safe because happens "inside" a suspend call and 
			// can't happen concurrently with a resume call
		}
		);

	// release batch processing so the task above will be executed
	FAtomThread::ProcessAllCommands();

	// wait for the command above to block atom processing
	SuspendEvent->Wait();
}

void FAtomThread::ResumeAtomThread()
{
	check(IsInGameThread());

	if (!GIsAtomThreadRunning)
	{
		return; // nothing to resume
	}

	if (--SuspendCount != 0)
	{
		return; // recursive scope
	}

	check(GIsAtomThreadSuspended.load(std::memory_order_relaxed));
	GIsAtomThreadSuspended.store(false, std::memory_order_release);

	check(!ResumeEvent->IsCompleted());
	ResumeEvent->Trigger();
}

#else // UE_ATOM_THREAD_AS_PIPE

FRunnable* FAtomThread::AtomThreadRunnable = nullptr;

/** The atom thread main loop */
void AtomThreadMain(FEvent* TaskGraphBoundSyncEvent)
{
	FTaskGraphInterface::Get().AttachToThread(ENamedThreads::AtomThread);
	FPlatformMisc::MemoryBarrier();

	// Inform main thread that the atom thread has been attached to the taskgraph and is ready to receive tasks
	if (TaskGraphBoundSyncEvent != nullptr)
	{
		TaskGraphBoundSyncEvent->Trigger();
	}

	FTaskGraphInterface::Get().ProcessThreadUntilRequestReturn(ENamedThreads::AtomThread);
	FPlatformMisc::MemoryBarrier();
}

FAtomThread::FAtomThread()
{
	TaskGraphBoundSyncEvent = FPlatformProcess::GetSynchEventFromPool(true);

	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddRaw(this, &FAtomThread::OnPreGarbageCollect);
	FCoreUObjectDelegates::GetPostGarbageCollect().AddRaw(this, &FAtomThread::OnPostGarbageCollect);

	FCoreUObjectDelegates::PreGarbageCollectConditionalBeginDestroy.AddRaw(this, &FAtomThread::OnPreGarbageCollect);
	FCoreUObjectDelegates::PostGarbageCollectConditionalBeginDestroy.AddRaw(this, &FAtomThread::OnPostGarbageCollect);
}

FAtomThread::~FAtomThread()
{
	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().RemoveAll(this);
	FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll(this);

	FCoreUObjectDelegates::PreGarbageCollectConditionalBeginDestroy.RemoveAll(this);
	FCoreUObjectDelegates::PostGarbageCollectConditionalBeginDestroy.RemoveAll(this);

	FPlatformProcess::ReturnSynchEventToPool(TaskGraphBoundSyncEvent);
	TaskGraphBoundSyncEvent = nullptr;
}

static int32 AtomThreadSuspendCount = 0;

void FAtomThread::SuspendAtomThread()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(SuspendAtomThread);
	check(FPlatformTLS::GetCurrentThreadId() == GGameThreadId);
	check(!GIsAtomThreadSuspended.Load() || GCVarSuspendAtomThread != 0);

	if (IsAtomThreadRunning())
	{
		// Make GC wait on the atom thread finishing processing
		FAtomCommandFence AtomFence;
		AtomFence.BeginFence();
		AtomFence.Wait();

		GIsAtomThreadSuspended = true;
		FPlatformMisc::MemoryBarrier();
	}
	check(!IsAtomThreadRunning());
}

void FAtomThread::ResumeAtomThread()
{
PRAGMA_DISABLE_DEPRECATION_WARNINGS

		check(FPlatformTLS::GetCurrentThreadId() == GGameThreadId);
	if (GIsAtomThreadSuspended.Load() && GCVarSuspendAtomThread == 0)
	{
		GIsAtomThreadSuspended = false;
		FPlatformMisc::MemoryBarrier();
	}
	ProcessAllCommands();

PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void FAtomThread::OnPreGarbageCollect()
{
	AtomThreadSuspendCount++;
	if (AtomThreadSuspendCount == 1)
	{
		SuspendAtomThread();
	}
}

void FAtomThread::OnPostGarbageCollect()
{
	AtomThreadSuspendCount--;
	if (AtomThreadSuspendCount == 0)
	{
		ResumeAtomThread();
	}
}

#if !UE_ATOM_THREAD_AS_PIPE

bool FAtomThread::Init()
{
	GIsAtomThreadRunning.store(true, std::memory_order_release);
	return true;
}

void FAtomThread::Exit()
{
	GIsAtomThreadRunning.store(false, std::memory_order_release);
}

#endif

uint32 FAtomThread::Run()
{
	LLM_SCOPE(ELLMTag::AtomMisc);
	SCOPED_NAMED_EVENT(FAtomThread_Run, FColor::Blue);

	FMemory::SetupTLSCachesOnCurrentThread();
	FPlatformProcess::SetupAtomThread();
	AtomThreadMain(TaskGraphBoundSyncEvent);
	FMemory::ClearAndDisableTLSCachesOnCurrentThread();
	return 0;
}

#endif // UE_ATOM_THREAD_AS_PIPE

void FAtomThread::SetUseThreadedAtom(const bool bInUseThreadedAtom)
{
	if (IsAtomThreadRunning() && !bInUseThreadedAtom)
	{
		UE_LOG(LogCriWareAtom, Error, TEXT("You cannot disable using threaded Atom once the thread has already begun running."));
	}
	else
	{
		bUseThreadedAtom = bInUseThreadedAtom;
	}
}

bool FAtomThread::IsUsingThreadedAtom()
{
	return bUseThreadedAtom;
}

#if UE_ATOM_THREAD_AS_PIPE

// Batching Atom commands allows to avoid the overhead of launching a task per command when resources are limited.
// We assume that resources are limited if the previous batch is not completed yet (potentially waiting for execution due to the CPU being busy
// with something else). Otherwise we don't wait until we collect a full batch.
struct FAtomAsyncBatcher
{
	using FWork = TUniqueFunction<void()>;
	TArray<FWork> WorkItems;

	UE::Tasks::FTask LastBatch;

	void Add(FWork&& Work)
	{
		check(IsInGameThread());

#if !WITH_EDITOR
		if (GCVarEnableAtomBatchProcessing)
		{
			if (WorkItems.Num() >= GBatchAtomAsyncBatchSize) // collected enough work
			{
				Flush();
			}
			WorkItems.Add(Forward<FWork>(Work));
		}
#else
		LastBatch = GAtomPipe.Launch(UE_SOURCE_LOCATION, MoveTemp(Work));
#endif
	}

	void Flush()
	{
		check(IsInGameThread());

		if (WorkItems.IsEmpty())
		{
			return;
		}

		LastBatch = GAtomPipe.Launch(TEXT("AtomBatch"),
			[WorkItems = MoveTemp(WorkItems)]() mutable
			{
				LLM_SCOPE_CRIWARE(ELLMTagCriWare::AtomMisc);

				for (FWork& Work : WorkItems)
				{
					Work();
				}
			},
			GAtomTaskPriority
		);
		WorkItems.Reset();
	}
};

#else // UE_ATOM_THREAD_AS_PIPE

struct FAtomAsyncBatcher
{
	FGraphEventArray DispatchEvent;
	int32 NumBatched = 0;


	FGraphEventArray* GetAsyncPrereq()
	{
		check(IsInGameThread());
#if !WITH_EDITOR
		if (GCVarEnableAtomBatchProcessing)
		{
			if (NumBatched >= GBatchAtomAsyncBatchSize || !DispatchEvent.Num() || !DispatchEvent[0].GetReference() || DispatchEvent[0]->IsComplete())
			{
				Flush();
			}
			if (DispatchEvent.Num() == 0)
			{
				check(NumBatched == 0);
				DispatchEvent.Add(FGraphEvent::CreateGraphEvent());
			}
			NumBatched++;
			return &DispatchEvent;
		}
#endif
		return nullptr;
	}

	void Flush()
	{
		check(IsInGameThread());
		if (NumBatched)
		{
			check(DispatchEvent.Num() && DispatchEvent[0].GetReference() && !DispatchEvent[0]->IsComplete());
			FGraphEventRef Dispatch = DispatchEvent[0];
			TFunction<void()> FlushAtomCommands = [Dispatch]()
			{
				TArray<FBaseGraphTask*> NewTasks;
				Dispatch->DispatchSubsequents(NewTasks);
			};

			FFunctionGraphTask::CreateAndDispatchWhenReady(MoveTemp(FlushAtomCommands), TStatId(), nullptr, ENamedThreads::AtomThread);

			DispatchEvent.Empty();
			NumBatched = 0;
		}
	}

};

#endif // UE_ATOM_THREAD_AS_PIPE

static FAtomAsyncBatcher GAtomAsyncBatcher;

TUniqueFunction<void()> FAtomThread::GetCommandWrapper(TUniqueFunction<void()> InFunction, const TStatId InStatId)
{
	if (GCVarEnableAtomCommandLogging == 1)
	{
		return[Function = MoveTemp(InFunction), InStatId]()
		{
#if !UE_ATOM_THREAD_AS_PIPE
			FTaskTagScope Scope(ETaskTag::EAtomThread);
#endif
			FScopeCycleCounter ScopeCycleCounter(InStatId);
			FAtomThread::SetCurrentAtomThreadStatId(InStatId);

			// Time the execution of the function
			const double StartTime = FPlatformTime::Seconds();

			// Execute the function
			Function();

			// Track the longest one
			const double DeltaTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
			if (DeltaTime > GetCurrentLongestTime())
			{
				SetLongestTimeAndId(InStatId, DeltaTime);
			}
		};
	}
	else
	{
		return[Function = MoveTemp(InFunction), InStatId]()
		{
#if !UE_ATOM_THREAD_AS_PIPE
			FTaskTagScope Scope(ETaskTag::EAtomThread);
#endif
			FScopeCycleCounter ScopeCycleCounter(InStatId);
			Function();
		};
	}
}

void FAtomThread::RunCommandOnAtomThread(TUniqueFunction<void()> InFunction, const TStatId InStatId)
{
#if UE_ATOM_THREAD_AS_PIPE

	TUniqueFunction<void()> CommandWrapper{ GetCommandWrapper(MoveTemp(InFunction), InStatId) };
	if (IsInAtomThread())
	{
		// it's atom-thread-safe so execute the command in-place
		CommandWrapper();
	}
	else if (IsInGameThread())
	{
		// batch commands to minimise game thread overhead
		GAtomAsyncBatcher.Add(MoveTemp(CommandWrapper));
	}
	// we are on an unknown thread
	else if (IsUsingThreadedAtom())
	{
		GAtomPipe.Launch(TEXT("AtomCommand"), MoveTemp(CommandWrapper), GAtomTaskPriority);
	}
	else
	{
		// the command must be executed on the game thread
		AsyncTask(ENamedThreads::GameThread, MoveTemp(CommandWrapper));
	}

#else

	if (IsInAtomThread())
	{
		// it's atom-thread-safe so execute the command in-place
		FScopeCycleCounter ScopeCycleCounter(InStatId);
		InFunction();
		return;
	}

	TUniqueFunction<void()> CommandWrapper{ GetCommandWrapper(MoveTemp(InFunction), InStatId) };
	if (IsInGameThread())
	{
		// batch commands to minimise game thread overhead
		FFunctionGraphTask::CreateAndDispatchWhenReady(MoveTemp(CommandWrapper), TStatId{}, GAtomAsyncBatcher.GetAsyncPrereq(), ENamedThreads::AtomThread);
	}
	// we are on an unknown thread
	else if (IsUsingThreadedAtom())
	{
		FFunctionGraphTask::CreateAndDispatchWhenReady(MoveTemp(CommandWrapper), TStatId{}, nullptr, ENamedThreads::AtomThread);
	}
	else
	{
		// the command must be executed on the game thread
		FFunctionGraphTask::CreateAndDispatchWhenReady(MoveTemp(CommandWrapper), TStatId{}, nullptr, ENamedThreads::GameThread);
	}

#endif
}

void FAtomThread::SetCurrentAtomThreadStatId(TStatId InStatId)
{
	FScopeLock Lock(&CurrentAtomThreadStatIdCS);
	CurrentAtomThreadStatId = InStatId;
}

FString FAtomThread::GetCurrentAtomThreadStatId()
{
	FScopeLock Lock(&CurrentAtomThreadStatIdCS);
#if STATS
	return FString(CurrentAtomThreadStatId.GetStatDescriptionANSI());
#else
	return FString(TEXT("NoStats"));
#endif
}

void FAtomThread::ResetAtomThreadTimers()
{
	FScopeLock Lock(&CurrentAtomThreadStatIdCS);
	LongestAtomThreadStatId = TStatId();
	LongestAtomThreadTimeMsec = 0.0;
}

void FAtomThread::SetLongestTimeAndId(TStatId NewLongestId, double LongestTimeMsec)
{
	FScopeLock Lock(&CurrentAtomThreadStatIdCS);
	LongestAtomThreadTimeMsec = LongestTimeMsec;
	LongestAtomThreadStatId = NewLongestId;
}

void FAtomThread::GetLongestTaskInfo(FString& OutLongestTask, double& OutLongestTaskTimeMs)
{
	FScopeLock Lock(&CurrentAtomThreadStatIdCS);
#if STATS
	OutLongestTask = FString(LongestAtomThreadStatId.GetStatDescriptionANSI());
#else
	OutLongestTask = FString(TEXT("NoStats"));
#endif
	OutLongestTaskTimeMs = LongestAtomThreadTimeMsec;
}

void FAtomThread::ProcessAllCommands()
{
	if (IsAtomThreadRunning())
	{
		GAtomAsyncBatcher.Flush();
	}
	else
	{
#if UE_ATOM_THREAD_AS_PIPE
		check(GAtomAsyncBatcher.WorkItems.IsEmpty());
#else
		check(FPlatformTLS::GetCurrentThreadId() == GGameThreadId);
#endif
	}
}

void FAtomThread::RunCommandOnGameThread(TUniqueFunction<void()> InFunction, const TStatId InStatId)
{
	if (IsAtomThreadRunning())
	{
		check(IsInAtomThread());
		FFunctionGraphTask::CreateAndDispatchWhenReady(MoveTemp(InFunction), InStatId, nullptr, ENamedThreads::GameThread);
	}
	else
	{
		check(IsInGameThread());
		FScopeCycleCounter ScopeCycleCounter(InStatId);
		InFunction();
	}
}

#if UE_ATOM_THREAD_AS_PIPE

FDelegateHandle FAtomThread::PreGC;
FDelegateHandle FAtomThread::PostGC;
FDelegateHandle FAtomThread::PreGCDestroy;
FDelegateHandle FAtomThread::PostGCDestroy;

void FAtomThread::StartAtomThread()
{
	check(IsInGameThread());

	check(!GIsAtomThreadRunning.load(std::memory_order_relaxed));

	if (!bUseThreadedAtom)
	{
		return;
	}

	PreGC = FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddStatic(&FAtomThread::SuspendAtomThread);
	PostGC = FCoreUObjectDelegates::GetPostGarbageCollect().AddStatic(&FAtomThread::ResumeAtomThread);

	PreGCDestroy = FCoreUObjectDelegates::PreGarbageCollectConditionalBeginDestroy.AddStatic(&FAtomThread::SuspendAtomThread);
	PostGCDestroy = FCoreUObjectDelegates::PostGarbageCollectConditionalBeginDestroy.AddStatic(&FAtomThread::ResumeAtomThread);

	check(!ResumeEvent.IsValid());
	ResumeEvent = MakeUnique<UE::Tasks::FTaskEvent>(UE_SOURCE_LOCATION);

	GIsAtomThreadRunning.store(true, std::memory_order_release);
}

void FAtomThread::StopAtomThread()
{
	if (!IsAtomThreadRunning())
	{
		return;
	}

	GAtomAsyncBatcher.Flush();
	GAtomAsyncBatcher.LastBatch.Wait();
	GAtomAsyncBatcher.LastBatch = UE::Tasks::FTask{}; // release the task as it can hold some references

	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().Remove(PreGC);
	FCoreUObjectDelegates::GetPostGarbageCollect().Remove(PostGC);
	FCoreUObjectDelegates::PreGarbageCollectConditionalBeginDestroy.Remove(PreGCDestroy);
	FCoreUObjectDelegates::PostGarbageCollectConditionalBeginDestroy.Remove(PostGCDestroy);

	GIsAtomThreadRunning.store(false, std::memory_order_release);

	check(ResumeEvent.IsValid());
	ResumeEvent->Trigger(); // every FTaskEvent must be triggered before destruction to pass the check for completion
	ResumeEvent.Reset();
}

FAtomCommandFence::~FAtomCommandFence()
{
	check(IsInGameThread());
	Fence.Wait();
}

void FAtomCommandFence::BeginFence()
{
	check(IsInGameThread());

	if (!IsAtomThreadRunning())
	{
		return;
	}

	Fence.Wait();
	FAtomThread::ProcessAllCommands();
	Fence = GAtomAsyncBatcher.LastBatch;
}

bool FAtomCommandFence::IsFenceComplete() const
{
	check(IsInGameThread());
	return Fence.IsCompleted();
}

/**
 * Waits for pending fence commands to retire.
 */
void FAtomCommandFence::Wait(bool bProcessGameThreadTasks) const
{
	check(IsInGameThread());
	Fence.Wait();
	Fence = UE::Tasks::FTask{}; // release the task as it can hold some references
}

#else // UE_ATOM_THREAD_AS_PIPE

void FAtomThread::StartAtomThread()
{
	check(FPlatformTLS::GetCurrentThreadId() == GGameThreadId);

	check(!IsAtomThreadRunning());
	check(!GIsAtomThreadSuspended);
	if (bUseThreadedAtom)
	{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		check(GAtomThread == nullptr);
PRAGMA_ENABLE_DEPRECATION_WARNINGS

		static uint32 ThreadCount = 0;
		check(!ThreadCount); // we should not stop and restart the atom thread; it is complexity we don't need.

		// Create the atom thread.
		AtomThreadRunnable = new FAtomThread();

PRAGMA_DISABLE_DEPRECATION_WARNINGS
		GAtomThread =
PRAGMA_ENABLE_DEPRECATION_WARNINGS
		FRunnableThread::Create(AtomThreadRunnable, *FName(NAME_AtomThread).GetPlainNameString(), 0, (GCVarAboveNormalAtomThreadPri == 0) ? TPri_BelowNormal : TPri_AboveNormal, FPlatformAffinity::GetAtomThreadMask());

		// Wait for atom thread to have taskgraph bound before we dispatch any tasks for it.
		((FAtomThread*)AtomThreadRunnable)->TaskGraphBoundSyncEvent->Wait();

		// ensure the thread has actually started and is idling
		FAtomCommandFence Fence;
		Fence.BeginFence();
		Fence.Wait();

		ThreadCount++;

		if (GCVarSuspendAtomThread != 0)
		{
			SuspendAtomThread();
		}
	}
}

void FAtomThread::StopAtomThread()
{
	check(FPlatformTLS::GetCurrentThreadId() == GGameThreadId);
	check(!GIsAtomThreadSuspended.Load() || GCVarSuspendAtomThread != 0);

	if (!IsAtomThreadRunning())
	{
		return;
	}

	FAtomCommandFence Fence;
	Fence.BeginFence();
	Fence.Wait();
	FGraphEventRef QuitTask = TGraphTask<FReturnGraphTask>::CreateTask(nullptr, ENamedThreads::GameThread).ConstructAndDispatchWhenReady(ENamedThreads::AtomThread);

	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_StopAtomThread);
		FTaskGraphInterface::Get().WaitUntilTaskCompletes(QuitTask, ENamedThreads::GameThread_Local);
	}

PRAGMA_DISABLE_DEPRECATION_WARNINGS
	// Wait for the atom thread to return.
	GAtomThread->WaitForCompletion();

	// Destroy the atom thread objects.
	delete GAtomThread;
	GAtomThread = nullptr;
PRAGMA_ENABLE_DEPRECATION_WARNINGS

	delete AtomThreadRunnable;
	AtomThreadRunnable = nullptr;
}

FAtomCommandFence::FAtomCommandFence()
	: FenceDoneEvent(nullptr)
{
}

FAtomCommandFence::~FAtomCommandFence()
{
	if (FenceDoneEvent)
	{
		FenceDoneEvent->Wait();

		FPlatformProcess::ReturnSynchEventToPool(FenceDoneEvent);
		FenceDoneEvent = nullptr;
	}
}

void FAtomCommandFence::BeginFence()
{
	if (IsAtomThreadRunning())
	{
		DECLARE_CYCLE_STAT(TEXT("FNullGraphTask.FenceAtomCommand"),
			STAT_FNullGraphTask_FenceAtomCommand,
			STATGROUP_TaskGraphTasks);

		CompletionEvent = TGraphTask<FNullGraphTask>::CreateTask(GAtomAsyncBatcher.GetAsyncPrereq(), ENamedThreads::GameThread).ConstructAndDispatchWhenReady(
			GET_STATID(STAT_FNullGraphTask_FenceAtomCommand), ENamedThreads::AtomThread);

		if (FenceDoneEvent)
		{
			FenceDoneEvent->Wait();

			FPlatformProcess::ReturnSynchEventToPool(FenceDoneEvent);
			FenceDoneEvent = nullptr;
		}

		FenceDoneEvent = FPlatformProcess::GetSynchEventFromPool(true);

		FTaskGraphInterface::Get().TriggerEventWhenTaskCompletes(FenceDoneEvent, CompletionEvent, ENamedThreads::GameThread, ENamedThreads::AtomThread);

		FAtomThread::ProcessAllCommands();
	}
	else
	{
		CompletionEvent = nullptr;
	}
}

bool FAtomCommandFence::IsFenceComplete() const
{
	FAtomThread::ProcessAllCommands();

	if (!CompletionEvent.GetReference() || CompletionEvent->IsComplete())
	{
		CompletionEvent = nullptr; // this frees the handle for other uses, the NULL state is considered completed
		return true;
	}

	check(IsAtomThreadRunning());

	return FenceDoneEvent->Wait(0);
}

/**
 * Waits for pending fence commands to retire.
 */
void FAtomCommandFence::Wait(bool bProcessGameThreadTasks) const
{
	FAtomThread::ProcessAllCommands();

	if (!IsFenceComplete()) // this checks the current thread
	{
		const double StartTime = FPlatformTime::Seconds();
		QUICK_SCOPE_CYCLE_COUNTER(STAT_FAtomCommandFence_Wait);

		bool bDone = false;

		do
		{
			if (FenceDoneEvent)
			{
				bDone = FenceDoneEvent->Wait(GAtomCommandFenceWaitTimeMs);
			}
			else
			{
				bDone = true;
			}

			if (bDone && FenceDoneEvent)
			{
				FPlatformProcess::ReturnSynchEventToPool(FenceDoneEvent);
				FenceDoneEvent = nullptr;
			}

			// Log how long we've been waiting for the atom thread:
			float ThisTime = FPlatformTime::Seconds() - StartTime;
			if (ThisTime > static_cast<float>(GAtomCommandFenceWaitTimeMs) / 1000.0f + SMALL_NUMBER)
			{
				if (GCVarEnableAtomCommandLogging == 1)
				{
					FString CurrentTask = FAtomThread::GetCurrentAtomThreadStatId();

					FString LongestTask;
					double LongestTaskTimeMs;
					FAtomThread::GetLongestTaskInfo(LongestTask, LongestTaskTimeMs);

					UE_LOG(LogCriWareAtom, Display, TEXT("Waited %.2f ms for atom thread. (Current Task: %s, Longest task: %s %.2f ms)"), ThisTime * 1000.0f, *CurrentTask, *LongestTask, LongestTaskTimeMs);
				}
				else
				{
					UE_LOG(LogCriWareAtom, Display, TEXT("Waited %f ms for atom thread."), ThisTime * 1000.0f);
				}
			}
		} while (!bDone);

		FAtomThread::ResetAtomThreadTimers();
	}
}

#endif // UE_ATOM_THREAD_AS_PIPE
