/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2017-2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareSequencer
 * File     : CriWareSequencer.cpp
 *
 ****************************************************************************/

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "ISequencer.h"
#include "ISequencerModule.h"

#include "CriWareSequencerPrivate.h"
#include "ICriWareSequencer.h"
#include "CriWareSequencerEditorStyle.h"
#include "AtomTrackEditor.h"

#include "CriWareDefines.h"

/**
 * CriWareSequencer module implementation (private)
 */
class FCriWareSequencerModule : public ICriWareSequencer
{
public:
	/** IModuleInterface */
	virtual void StartupModule() override
	{
		FCriWareSequencerEditorStyle::Get();

		ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
		AtomTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FAtomTrackEditor::CreateTrackEditor));
	}

	virtual void ShutdownModule() override
	{
		FCriWareSequencerEditorStyle::Destroy();

		ISequencerModule* SequencerModulePtr = FModuleManager::Get().GetModulePtr<ISequencerModule>("Sequencer");
		if (SequencerModulePtr)
		{
			SequencerModulePtr->UnRegisterTrackEditor(AtomTrackCreateEditorHandle);
		}
	}

private:
	/** Registered delegate handles */
	FDelegateHandle AtomTrackCreateEditorHandle;
};

IMPLEMENT_MODULE(FCriWareSequencerModule, CriWareSequencer);
