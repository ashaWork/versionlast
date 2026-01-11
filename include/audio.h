/* Start Header ************************************************************************/
/*!
\file       audio.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100% of the file
\par        cheeqing.tan@digipen.edu
\date       September, 20th, 2025
\brief      This header file declares the AudioHandler class, a singleton wrapper around
			the FMOD library for managing sound in the project. It provides function
			prototypes for initializing and shutting down the audio system, loading sounds,
			playing them and controlling playback features such as pause, stop, mute,
			volume and pitch. The soundID enum is defined to categorize different sounds.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

#include <../lib/FMOD/fmod.hpp>
//#include <../lib/FMOD/fmod_errors.h>
#include <vector>
#include <string>
#include <iostream>

#include "AudioUtility.h"
#include "ResourceManager.h"
#include "performance.h"
#include "Component.h"
#include "GameObjectManager.h"

//enum class AudioState {
//	Stopped,
//	Playing,
//	Paused
//};

//enum class soundID : int {
//	water = 0,
//	shoot,
//	fire,
//	bg,
//	// keep this as the last, for getting the number of sound ids
//	count
//};

//struct FadeInfo {
//	bool isFading = false;
//	bool isFadingIn = false;
//	float startVolume = 0.0f;
//	float targetVolume = 0.0f;
//	float fadeDuration = 0.0f;
//	float fadeTimer = 0.0f;
//
//	float defaultFadeInDuration = 1.0f;
//	float defaultFadeOutDuration = 1.0f;
//};


class AudioHandler {
public:
	// get the singleton instance
	static AudioHandler& getInstance(); // make sure only 1 instance is created

	// check for FMOD errors (future: can change to logging instead of exit(-1))
	void init();
	// initialize the audio system and load sounds
	void update(float deltaTime);
	// release all sounds and shut down the audio system (will shutdown automatically when dtor called)
	void shutdown(); // no need to manually call, dtor will call this automatically

	// load the sound from file path, if loop is true, sound will loop until stopped (default false)
	// now done in ResourceManager
	// bool loadSound(soundID id, const std::string& filePath, bool loop = false);

	// play the sound, return the channel pointer if successful, nullptr if failed
	FMOD::Channel* playSound(AudioChannel* audio);

	// call this to PAUSE / RESUME sound
	/*void toggleSoundPause(AudioChannel* audio);*/

	// temp to just pause/resume all audio for game obj
	void pauseAll(GameObjectManager& manager);
	void resumeAll(GameObjectManager& manager);

	void pauseSound(AudioChannel* audio);

	void resumeSound(AudioChannel* audio);
	// use this to PAUSE / RESUME ALL sounds
	//void toggleAllSoundPause(); // DO NOT USE!!
	// use this to check whether sound is paused
	bool isSoundPaused(AudioChannel* audio);

	// to stop a particular sound
	void stopSound(AudioChannel* audio);
	// to check whether channel still alive (ie. sound finished playing)
	bool isSoundPlaying(AudioChannel* audio);

	// mute specific sound
	void muteSound(AudioChannel* audio);
	// mute all sounds
	//void muteAllSound(); // DO NO USE!!
	// unmute specific sound
	void unmuteSound(AudioChannel* audio);
	// unmute all sounds
	//void unmuteAllSound(); // DO NOT USE!!
	// increase by 0.1f
	void increaseSoundVolume(AudioChannel* audio);
	// decrease by 0.1f
	void decreaseSoundVolume(AudioChannel* audio);
	// set a specific volume
	void setSoundVolume(AudioChannel* audio, float volume); // 0.0f (muted) to 1.0f (full)

	// set a specific pitch
	void setSoundpitch(AudioChannel* audio, float pitch); // 0.5f (half speed) to 2.0f (double speed)

	//pause and resume
	//void pauseAll(); // DO NOT USE!!
	//void resumeAll(); // DO NOT USE!!

	void fadeIn(AudioChannel* audio, float targetVolume = 1.0f, float duration = 1.0f);
	void fadeOut(AudioChannel* audio, float duration = 1.0f);
	bool isFading(AudioChannel* audio) const;
	void stopFade(AudioChannel* audio);

	//setters for fadeing
	void setFadeInDuration(AudioChannel* audio, float duration);
	void setFadeOutDuration(AudioChannel* audio, float duration);
	void setFadeVolume(AudioChannel* audio, float volume);
	//getters
	float getFadeInDuration(AudioChannel* audio) const;
	float getFadeOutDuration(AudioChannel* audio) const;
	float getFadeTargetVolume(AudioChannel* audio) const;
	float getFadeProgress(AudioChannel* audio) const;
	bool isFadingIn(AudioChannel* audio) const;
	bool isFadingOut(AudioChannel* audio) const;

	// perform error check for FMOD functions
	// moved to AudioUtility.h
	//static void FMODErrorCheck(FMOD_RESULT result);

private:
	AudioHandler() : system(nullptr) {}
	~AudioHandler() { shutdown(); } // auto shutdown once instance is destroyed

	// helper function to convert soundID to file path
	// currently hardcoded the path, to change once deserialization is implemented
	//std::string soundIDToFilePath(soundID id);

	//void updateFades(float deltaTime);

	FMOD::System* system;

	/* Now all audio components will have its own channel, this audioHandle can no longer manage all of the channels */
	//std::vector<FMOD::Sound*> sounds;
	//std::vector<FMOD::Channel*> channels;
	//std::vector<FadeInfo> fadeInfo;
};