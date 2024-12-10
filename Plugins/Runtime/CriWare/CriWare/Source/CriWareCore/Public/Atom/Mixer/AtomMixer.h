
//
// Notes: 
// 
// Like Unreal Audio Mixer conterpart, Atom Mixer handles the input and output sources and voices for an Atom runtime,
// but real mixage is performed by Atom engine. 
// 
// DSP patches from Unreal Audio, that are not dependent to a specific audio rngine,
// are used to make some buffered sources compatible between Atom and Unreal Audio. 
//

#pragma once

#include "CoreMinimal.h"

CRIWARECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogCriWareAtomMixer, Log, All);
CRIWARECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogCriWareAtomMixerDebug, Warning, All);

#ifndef ATOM_MIXER_ENABLE_DEBUG_MODE
// This define enables a bunch of more expensive debug checks and logging capabilities that are intended to be off most of the time even in debug builds of game/editor.
#if (UE_BUILD_SHIPPING || UE_BUILD_TEST)
#define ATOM_MIXER_ENABLE_DEBUG_MODE 0
#else
#define ATOM_MIXER_ENABLE_DEBUG_MODE 1
#endif
#endif

// Enable debug checking for audio mixer
#if ATOM_MIXER_ENABLE_DEBUG_MODE
#define ATOM_MIXER_CHECK(expr) ensure(expr)
#define ATOM_MIXER_CHECK_GAME_THREAD(_MixerRuntime)	(_MixerRuntime->CheckAtomThread())
#define ATOM_MIXER_CHECK_AUDIO_THREAD(_MixerRuntime)	(_MixerRuntime->CheckAudioRenderingThread())
#else
#define ATOM_MIXER_CHECK(expr)
#define ATOM_MIXER_CHECK_GAME_THREAD(_MixerRuntime)
#define ATOM_MIXER_CHECK_AUDIO_THREAD(_MixerRuntime)
#endif
