/* Start Header ************************************************************************/
/*!
\file       audio.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100% of the file
\par        cheeqing.tan@digipen.edu
\date       September, 20th, 2025
\brief      This source file implements the AudioHandler singleton class, which wraps
			FMOD functionality for managing audio in the project. It provides methods
			to initialize, update and shut down the audio system, as well as to load,
			play, pause, stop, mute and adjust volume or pitch of sounds identified by
			the soundID enum.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "audio.h"
#include <../lib/FMOD/fmod_errors.h>

// get the singleton instance
AudioHandler& AudioHandler::getInstance() {
	static AudioHandler instance;
	return instance;
}

// check for FMOD errors (future: can change to logging instead of exit(-1))
//void AudioHandler::FMODErrorCheck(FMOD_RESULT result) {
//    if (result != FMOD_OK) {
//        std::cerr << "FMOD error: " << FMOD_ErrorString(result) << std::endl;
//		exit(-1); // instead of terminate, can change to logging the errors
//    }
//}

//^might have to move this to a utility file if i wanna use in resource manager
//EDIT: moved to AudioUtility.h

// initialize the audio system and load sounds
void AudioHandler::init() {
	FMODErrorCheck(FMOD::System_Create(&system));

	FMODErrorCheck(system->init(36, FMOD_INIT_NORMAL, nullptr));

	ResourceManager::getInstance().init(system); // initialize resource manager with fmod system

	//sounds.resize(static_cast<size_t>(soundID::count), nullptr);
	//channels.resize(static_cast<size_t>(soundID::count), nullptr);
	//fadeInfo.resize(static_cast<size_t>(soundID::count));

	// only load bg for now
	//FMOD::Sound* sound = ResourceManager::getInstance().getSound("assets/audio/bg.wav", true);

	//for (int i = 0; i < static_cast<int>(soundID::count); ++i) {
	//	soundID id = static_cast<soundID>(i);	
	//	std::string path = soundIDToFilePath(id);

	//	/*if (id == soundID::bg && !path.empty() && !loadSound(id, path, true)) {
	//		std::cerr << "Failed to load sound: " << path << std::endl;
	//	}
	//	else if (!path.empty() && !loadSound(id, path)) {
	//		std::cerr << "Failed to load sound: " << path << std::endl;
	//	}*/

	//	// loading sounds now handled in ResourceManager
	//	if (!path.empty()) {
	//		bool loop = (id == soundID::bg);
	//		FMOD::Sound* sound = ResourceManager::getInstance().getSound(path, loop);
	//		if (!sound) {
	//			std::cerr << "Failed to load sound: " << path << std::endl;
	//		}
	//	}

	//}
}

// update the audio system each frame and set inactive channel to nullptr
void AudioHandler::update(float deltaTime) {
	(void)deltaTime;
	//record current time for performance tracking
	auto start = std::chrono::high_resolution_clock::now();
	if (system) {
		system->update();
	}
	//updateFades(deltaTime);
	//for (size_t i = 0; i < channels.size(); ++i) {
	//	if (channels[i]) {
	//		bool isPlaying = false;
	//		channels[i]->isPlaying(&isPlaying);  // NEED to call fmoderrorcheck() here once exit(-1) change to logging
	//		if (!isPlaying) {
	//			channels[i] = nullptr;
	//			fadeInfo[i].isFading = false;
	//		}
	//	}
	//}
	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	g_SystemTimers.push_back({ "Audio", ms }); //saving timing for UI output
}

// release all sounds and shut down the audio system (will shutdown automatically when dtor called)
void AudioHandler::shutdown() {
	/*for (FMOD::Sound* sound : sounds) {
		if (sound) {
			sound->release();
		}
	}

	sounds.clear();*/
	//ResourceManager will handle releasing sounds now

	/*channels.clear();
	fadeInfo.clear();*/

	if (system) {
		system->close();
		system->release();
		system = nullptr;
	}
}


/* Sound Management */
// now done in ResourceManager
// load the sound from file path, if loop is true, sound will loop until stopped (default false)
//bool AudioHandler::loadSound(soundID id, const std::string& filePath, bool loop) {
//	if (!system) {
//		std::cerr << "Error! Audio system not initialized..." << std::endl;
//		return false;
//	}
//
//	size_t pos = static_cast<size_t>(id);
//	if (sounds[pos] != nullptr) return true; // means sounds alr loaded, jic called multiple times
//
//	FMOD::Sound* sound = nullptr;
//	FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT; // by default, sound stops after playing once
//	FMODErrorCheck(system->createSound(filePath.c_str(), mode, nullptr, &sound));
//
//	sounds[pos] = sound;
//
//	return true;
//}

// play the sound, return the channel pointer if successful, nullptr if failed
FMOD::Channel* AudioHandler::playSound(AudioChannel* audio) {
	if (!system) {
		std::cerr << "Error! Audio system not initialized..." << std::endl;
		return nullptr;
	}

	//std::string path = soundIDToFilePath(id);
	if (audio->audioFile.empty()) {
		std::cerr << "Error! Sound file path is empty..." << std::endl;
		return nullptr;
	}
	/*if (sounds[pos] == nullptr) {
		std::cerr << "Error! Sound not loaded..." << std::endl;
		return nullptr;
	}*/
	//bool loop = (id == soundID::bg);
	FMOD::Sound* sound = ResourceManager::getInstance().getSound(audio->audioFile);
	if (!sound) {
		std::cerr << "Error! Sound could not be loaded: " << audio->audioFile << std::endl;
		return nullptr;
	}

	FMODErrorCheck(sound->setMode(audio->loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF));

	FMOD::Channel* channel = nullptr;
	//FMODErrorCheck(system->playSound(sounds[pos], nullptr, false, &channel));
	FMODErrorCheck(system->playSound(sound, nullptr, false, &channel));
	if (channel) {
		channel->setVolume(audio->volume);
		channel->setPitch(audio->pitch);
		//channels[pos] = channel; // store this for controlling ltr
		/*channels[static_cast<size_t>(id)] = channel;
		fadeInfo[static_cast<size_t>(id)].isFading = false;*/
		audio->channel = channel;
		audio->fadeInfo.isFading = false;
		std::cout << "[AUDIO] Playing: " << audio->audioFile << std::endl;
	}

	return channel;
}

void AudioHandler::pauseAll(GameObjectManager& manager) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* obj : gameObjects) {
		if (!obj->hasComponent<AudioComponent>()) continue;
		AudioComponent* ac = obj->getComponent<AudioComponent>();

		for (auto& pair : ac->audioChannels) {

			AudioChannel& ch = pair.second;

			if (ch.state == AudioState::Playing && ch.channel) {
				pauseSound(&ch);
				ch.state = AudioState::Paused;
			}

		}

	}
}

void AudioHandler::resumeAll(GameObjectManager& manager) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* obj : gameObjects) {
		if (!obj->hasComponent<AudioComponent>()) continue;
		AudioComponent* ac = obj->getComponent<AudioComponent>();

		for (auto& pair : ac->audioChannels) {

			AudioChannel& ch = pair.second;

			if (ch.state == AudioState::Paused && ch.channel) {
				resumeSound(&ch);
				ch.state = AudioState::Playing;
			}

		}

	}
}

// PAUSE / RESUME a particular sound
//void AudioHandler::toggleSoundPause(AudioChannel* audio) {
//	//size_t pos = static_cast<size_t>(id);
//	bool isPaused = false;
//
//	/*if (channels[pos]) {*/
//	if(audio->channel){
//		FMODErrorCheck(audio->channel->getPaused(&isPaused));
//		// if paused, resume playing, if playing, pause
//		FMODErrorCheck(audio->channel->setPaused(!isPaused));
//	}
//}

// PAUSE sound
void AudioHandler::pauseSound(AudioChannel* audio) {
	if (audio->channel) {
		FMODErrorCheck(audio->channel->setPaused(true));
	}
}

// RESUME sound
void AudioHandler::resumeSound(AudioChannel* audio) {
	if (audio->channel) {
		FMODErrorCheck(audio->channel->setPaused(false));
	}
}


// pause / resume all sounds
//void AudioHandler::toggleAllSoundPause() {
//	for (size_t i = 0; i < channels.size(); ++i) {
//		if (channels[i]) {
//			bool isPaused = false;
//			FMODErrorCheck(channels[i]->getPaused(&isPaused));
//			FMODErrorCheck(channels[i]->setPaused(!isPaused));
//		}
//	}
//}

// check whether sound is paused
bool AudioHandler::isSoundPaused(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);
	bool isPaused = false;

	/*if (channels[pos]) {*/
	if (audio->channel) {
		FMODErrorCheck(audio->channel->getPaused(&isPaused));
		return isPaused;
	}

	return false;
}

// stop a particular sound
void AudioHandler::stopSound(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);

	/*if (channels[pos]) {*/
	if (audio->channel) {
		FMODErrorCheck(audio->channel->stop());
		audio->channel = nullptr;
		audio->fadeInfo.isFading = false;
	}
}

// check whether channel still alive (ie. sound finished playing)
bool AudioHandler::isSoundPlaying(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);

	/*if (channels[pos]) {*/
	if (audio->channel) {
		bool isPlaying = false;
		FMODErrorCheck(audio->channel->isPlaying(&isPlaying));
		return isPlaying;
	}
	return false;
}

// mute specific sound
void AudioHandler::muteSound(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);

	/*if (channels[pos]) {*/
	if (audio->channel) {
		FMODErrorCheck(audio->channel->setMute(true));
	}
}

// mute all sounds
//void AudioHandler::muteAllSound() {
//	for (size_t i = 0; i < channels.size(); ++i) {
//		if (channels[i]) {
//			FMODErrorCheck(channels[i]->setMute(true));
//		}
//	}
//}

// unmute specific sound
void AudioHandler::unmuteSound(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);
	/*if (channels[pos]) {*/
	if (audio->channel) {
		FMODErrorCheck(audio->channel->setMute(false));
	}
}

// unmute all sounds
//void AudioHandler::unmuteAllSound() {
//	for (size_t i = 0; i < channels.size(); ++i) {
//		if (channels[i]) {
//			FMODErrorCheck(channels[i]->setMute(false));
//		}
//	}
//}

// increase by 0.1f
void AudioHandler::increaseSoundVolume(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);

	if (audio->channel) {
		float volume = 0.0f;
		FMODErrorCheck(audio->channel->getVolume(&volume));
		volume += 0.1f;
		if (volume > 1.0f) volume = 1.0f; // cap at 1.0f
		FMODErrorCheck(audio->channel->setVolume(volume));
	}
}

// decrease by 0.1f
void AudioHandler::decreaseSoundVolume(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);

	if (audio->channel) {
		float volume = 0.0f;
		FMODErrorCheck(audio->channel->getVolume(&volume));
		volume -= 0.1f;
		if (volume < 0.0f) volume = 0.0f; // cap at 0.0f
		FMODErrorCheck(audio->channel->setVolume(volume));
	}
}

// set volume of a sound to a specific value between 0.0f (muted) to 1.0f (full)
void AudioHandler::setSoundVolume(AudioChannel* audio, float volume) {
	//size_t pos = static_cast<size_t>(id);

	if (audio->channel)
		FMODErrorCheck(audio->channel->setVolume(volume));
}

// set a specific pitch between 0.5f (half speed) to 2.0f (double speed)
void AudioHandler::setSoundpitch(AudioChannel* audio, float pitch) {
	//size_t pos = static_cast<size_t>(id);

	if (audio->channel)
		FMODErrorCheck(audio->channel->setPitch(pitch));
}

void AudioHandler::fadeIn(AudioChannel* audio, float targetVolume, float duration) {
	//size_t pos = static_cast<size_t>(id);

	if (!audio->channel) {
		std::cerr << "Error! Cannot fade in - channel not active..." << std::endl;
		return;
	}

	FMODErrorCheck(audio->channel->setVolume(0.0f));

	audio->fadeInfo.isFading = true;
	audio->fadeInfo.isFadingIn = true;
	audio->fadeInfo.startVolume = 0.0f;
	audio->fadeInfo.targetVolume = targetVolume;
	audio->fadeInfo.fadeDuration = duration;
	audio->fadeInfo.fadeTimer = 0.0f;

	/*fadeInfo[pos].isFading = true;
	fadeInfo[pos].isFadingIn = true;
	fadeInfo[pos].startVolume = 0.0f;
	fadeInfo[pos].targetVolume = targetVolume;
	fadeInfo[pos].fadeDuration = duration;
	fadeInfo[pos].fadeTimer = 0.0f;*/
}

void AudioHandler::fadeOut(AudioChannel* audio, float duration) {
	//size_t pos = static_cast<size_t>(id);

	if (!audio->channel) {
		std::cerr << "Error! Cannot fade out - channel not active..." << std::endl;
		return;
	}

	float currentVolume = 0.0f;
	FMODErrorCheck(audio->channel->getVolume(&currentVolume));

	audio->fadeInfo.isFading = true;
	audio->fadeInfo.isFadingIn = false;
	audio->fadeInfo.startVolume = currentVolume;
	audio->fadeInfo.targetVolume = 0.f;
	audio->fadeInfo.fadeDuration = duration;
	audio->fadeInfo.fadeTimer = 0.f;

	/*fadeInfo[pos].isFading = true;
	fadeInfo[pos].isFadingIn = false;
	fadeInfo[pos].startVolume = currentVolume;
	fadeInfo[pos].targetVolume = 0.0f;
	fadeInfo[pos].fadeDuration = duration;
	fadeInfo[pos].fadeTimer = 0.0f;*/
}

// check if a sound is  fading
bool AudioHandler::isFading(AudioChannel* audio) const {
	//size_t pos = static_cast<size_t>(id);
	return audio->fadeInfo.isFading;
}

// stop any active fade for a sound
void AudioHandler::stopFade(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);
	audio->fadeInfo.isFading = false;
}

//void AudioHandler::updateFades(float deltaTime) {
//
//	//if (system) {
//	//	system->getSoftwareFormat(&rate, nullptr, nullptr);
//	//}
//
//	for (size_t i = 0; i < fadeInfo.size(); ++i) {
//		if (!fadeInfo[i].isFading || !channels[i]) {
//			continue;
//		}
//
//
//		fadeInfo[i].fadeTimer += deltaTime;
//
//		float progress = fadeInfo[i].fadeTimer / fadeInfo[i].fadeDuration;
//		if (progress > 1.0f) progress = 1.0f;
//
//		float currentVolume;
//		if (fadeInfo[i].isFadingIn) {
//			currentVolume = fadeInfo[i].startVolume + (fadeInfo[i].targetVolume - fadeInfo[i].startVolume) * progress;
//			std::cerr << "sound fading" << currentVolume << std::endl;
//		}
//		else {
//			currentVolume = fadeInfo[i].startVolume + (fadeInfo[i].targetVolume - fadeInfo[i].startVolume) * progress;
//		}
//
//		FMODErrorCheck(channels[i]->setVolume(currentVolume));
//
//		if (progress >= 1.0f) {
//			fadeInfo[i].isFading = false;
//
//			if (!fadeInfo[i].isFadingIn && fadeInfo[i].targetVolume == 0.0f) {
//				channels[i]->stop();
//				channels[i] = nullptr;
//			}
//		}
//	}
//}

/* HELPER FUNCTION */
// helper function to convert soundID to file path
//std::string AudioHandler::soundIDToFilePath(soundID id) {
//	// for now file paths are hardcoded, to change once deserialization is implemented
//	switch (id) {
//	case soundID::water:
//		return "assets/audio/water.wav";
//	case soundID::shoot:
//		return "assets/audio/underwater.wav";
//	case soundID::fire:
//		return "assets/audio/fire.wav";
//	case soundID::bg:
//		return "assets/audio/bg.wav";
//	default:
//		return "";
//	}
//}

//pause all sounds
//void AudioHandler::pauseAll()
//{
//	if (!system) return;
//
//	for (auto channel : channels) {
//		if (channel) {
//			bool isPlaying = false;
//			channel->isPlaying(&isPlaying);
//			if (isPlaying) {
//				channel->setPaused(true);
//			}
//		}
//	}
//}

//resume all sounds
//void AudioHandler::resumeAll() {
//	if (!system) return;
//
//	for (auto channel : channels) {
//		if (channel) {
//			bool isPaused = false;
//			channel->getPaused(&isPaused);
//			if (isPaused) {
//				channel->setPaused(false);
//			}
//		}
//	}
//}


// Setters for audiio fading
void AudioHandler::setFadeInDuration(AudioChannel* audio, float duration) {
	//size_t pos = static_cast<size_t>(id);

	//if (pos < fadeInfo.size() && duration > 0.0f) {
	audio->fadeInfo.defaultFadeInDuration = duration;
	//}
}

void AudioHandler::setFadeOutDuration(AudioChannel* audio, float duration) {
	//size_t pos = static_cast<size_t>(id);

	//if (pos < fadeInfo.size() && duration > 0.0f) {
	audio->fadeInfo.defaultFadeOutDuration = duration;
	//}
}

void AudioHandler::setFadeVolume(AudioChannel* audio, float volume) {
	//size_t pos = static_cast<size_t>(id);
	//if (pos < fadeInfo.size()) {
	audio->fadeInfo.targetVolume = std::max(0.0f, std::min(volume, 1.0f)); // clamp between 0 and 1
	//}
}

// Getters
float AudioHandler::getFadeInDuration(AudioChannel* audio) const {
	//size_t pos = static_cast<size_t>(id);
	//return (pos < fadeInfo.size()) ? fadeInfo[pos].defaultFadeInDuration : 1.0f;
	return audio->fadeInfo.defaultFadeInDuration;
}

float AudioHandler::getFadeOutDuration(AudioChannel* audio) const {
	//size_t pos = static_cast<size_t>(id);
	//return (pos < fadeInfo.size()) ? fadeInfo[pos].defaultFadeOutDuration : 1.0f;
	return audio->fadeInfo.defaultFadeOutDuration;
}

float AudioHandler::getFadeTargetVolume(AudioChannel* audio) const {
	//size_t pos = static_cast<size_t>(id);
	//return (pos < fadeInfo.size()) ? fadeInfo[pos].targetVolume : 0.0f;
	return audio->fadeInfo.targetVolume;
}

float AudioHandler::getFadeProgress(AudioChannel* audio) const {
	//size_t pos = static_cast<size_t>(id);
	if (audio->fadeInfo.isFading && audio->fadeInfo.fadeDuration > 0.0f) {
		return audio->fadeInfo.fadeTimer / audio->fadeInfo.fadeDuration;
	}
	return 0.0f;
}

bool AudioHandler::isFadingIn(AudioChannel* audio) const {
	//size_t pos = static_cast<size_t>(id);
	return audio->fadeInfo.isFading && audio->fadeInfo.isFadingIn;
}

bool AudioHandler::isFadingOut(AudioChannel* audio) const {
	//size_t pos = static_cast<size_t>(id);
	return audio->fadeInfo.isFading && !audio->fadeInfo.isFadingIn;
}