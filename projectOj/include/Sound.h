#pragma once

class Sound {
private:
    static int soundCounter;  // Counter for unique sound aliases
    
public:
    static void playGunshot();
    static void init();
    static void cleanup();
};

