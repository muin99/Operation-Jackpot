#include "Sound.h"
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif
#include <string>
#include <sstream>

// Initialize static counter
int Sound::soundCounter = 0;

void Sound::init() {
    soundCounter = 0;
}

void Sound::playGunshot() {
#ifdef _WIN32
    // Create a unique alias for this sound instance
    soundCounter++;
    std::stringstream ss;
    ss << "gunshot" << soundCounter;
    std::string alias = ss.str();
    
    // Open and play the MP3 file with unique alias
    std::string openCmd = "open \"assets/audio/gunshot.mp3\" type mpegvideo alias " + alias;
    if (mciSendStringA(openCmd.c_str(), NULL, 0, NULL) == 0) {
        // Play the sound (async - returns immediately, allows overlapping sounds)
        std::string playCmd = "play " + alias;
        mciSendStringA(playCmd.c_str(), NULL, 0, NULL);
    } else {
        // If MP3 fails, try as WAV file (in case it's actually a WAV)
        openCmd = "open \"assets/audio/gunshot.mp3\" type waveaudio alias " + alias;
        if (mciSendStringA(openCmd.c_str(), NULL, 0, NULL) == 0) {
            std::string playCmd = "play " + alias;
            mciSendStringA(playCmd.c_str(), NULL, 0, NULL);
        }
    }
    
    // Reset counter if it gets too large (prevent memory issues)
    // This also helps prevent too many open aliases
    if (soundCounter > 100) {
        // Clean up old aliases (keep last 50)
        for (int i = 1; i <= soundCounter - 50; i++) {
            std::stringstream cleanupSs;
            cleanupSs << "close gunshot" << i;
            mciSendStringA(cleanupSs.str().c_str(), NULL, 0, NULL);
        }
        soundCounter = 50;
    }
#endif
}

void Sound::cleanup() {
#ifdef _WIN32
    // Close all gunshot aliases (cleanup all instances)
    for (int i = 1; i <= soundCounter && i <= 1000; i++) {
        std::stringstream ss;
        ss << "close gunshot" << i;
        mciSendStringA(ss.str().c_str(), NULL, 0, NULL);
    }
    soundCounter = 0;
#endif
}

