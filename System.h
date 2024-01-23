#ifndef CHIPMU_SYSTEM_H
#define CHIPMU_SYSTEM_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <chrono>
#include <SDL.h>
#include <SDL_mixer.h>

class System {
public:
    explicit System(const std::string& game);
    ~System();
    void Start();
    void Draw();
    bool RunCycle();
private:
    void HandleMath(uint8_t type, uint8_t* x, const uint8_t* y);
    void HandleMisc(uint8_t type, uint8_t* x);

    SDL_Window* window_;
    SDL_Surface* surface_;

    //ram
    uint8_t memory_[4096];

    //general purpose registers + carry flag
    uint8_t registers_[16];

    //index register
    uint16_t index_;

    //program counter for something
    uint16_t counter_;

    //display buffer
    uint8_t* display_;

    //count at 60 hz- counts down when >0
    //buzzer sounds at sound = 0
    uint8_t delay_timer_;
    uint8_t sound_timer_;

    //for goto-esque op codes
    uint16_t stack_[16];

    //keymap to store key states
    bool keys_[16];

    bool last_keys_[16];
    int key_flag; //-2 - not waiting, -1 - waiting press, >=0 waiting release

    Mix_Chunk* buzz_;

    bool CheckInput();
};

#endif //CHIPMU_SYSTEM_H
