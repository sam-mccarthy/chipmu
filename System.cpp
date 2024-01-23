#include "System.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <chrono>
#include <SDL.h>
#include <SDL_mixer.h>

System::System(const std::string& game) : memory_{}, registers_{}, stack_{}, keys_{} {
    window_ = nullptr;
    surface_ = nullptr;
    buzz_ = nullptr;
    index_ = 0;
    counter_ = 0x200;
    delay_timer_ = 0;
    sound_timer_ = 0;
    display_ = memory_ + 0xF00;

    uint8_t font[5 * 16] = {0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                            0x20, 0x60, 0x20, 0x20, 0x70, // 1
                            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                            0xF0, 0x80, 0xF0, 0x80, 0x80}; // F
    memcpy(memory_ + 0x50, font, 5 * 16);

    std::ifstream file(game, std::ios::in|std::ios::binary);
    if(file.is_open()){
        char buffer[4096 - 512]{};

        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        file.read(buffer, size);
        memcpy(memory_ + 512, buffer, size);
    } else {
        printf("Problem opening file");
    }

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else {
        window_ = SDL_CreateWindow("chipmu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN);
        if(window_ == nullptr){
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        } else {
            surface_ = SDL_GetWindowSurface(window_);
        }
    }
}

System::~System() {
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void System::Start() {
    auto timer = std::chrono::steady_clock::now();

    uint64_t frames = 0;
    bool quit = false;
    while(!quit){
        uint64_t start = SDL_GetPerformanceCounter();
        if(std::chrono::steady_clock::now() - timer > std::chrono::microseconds(1666)){
            if(delay_timer_ > 0)
                delay_timer_--;
            if(sound_timer_ > 0) {
                sound_timer_--;
                if(sound_timer_ == 0){
                    //buzz =
                }
            }

            timer = std::chrono::steady_clock::now();
        }

        bool draw = false;
        for(int i = 0; i < 5 * 17; i++){
            quit = quit || CheckInput();
            draw = draw || RunCycle();
        }

        if(draw)
            Draw();
        uint64_t end = SDL_GetPerformanceCounter();
        float elapsed = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        SDL_Delay(floor(17 - elapsed));

        frames++;
    }
}

void System::Draw() {
    SDL_FillRect(surface_, nullptr, SDL_MapRGB(surface_->format, 0x0, 0xFF, 0x0));
    for(int y = 0; y < 32; y++){
        for(int x = 0; x < 64; x++){
            int index = y * 8 + x / 8;
            uint8_t color = (display_[index] & (0b10000000 >> (x % 8))) ? 0xFF : 0x00;
            SDL_Rect rect{x * 10, y * 10, 10, 10};

            SDL_FillRect(surface_, &rect, SDL_MapRGB(surface_->format, color, color, color));
        }
    }
    SDL_UpdateWindowSurface(window_);
}

bool System::CheckInput(){
    memcpy(last_keys_, keys_, 16);

    SDL_Event e;
    while(SDL_PollEvent(&e)){
        switch(e.type){
            case SDL_QUIT:
                return true;
            case SDL_KEYDOWN: {
                int code = e.key.keysym.sym;
                if (code > 47 && code < 58)
                    keys_[code - 48] = true;
                else if (code > 96 && code < 123)
                    keys_[code - 97 + 10] = true;
                break;
            }
            case SDL_KEYUP: {
                int code = e.key.keysym.sym;
                if (code > 47 && code < 58)
                    keys_[code - 48] = false;
                else if (code > 96 && code < 123)
                    keys_[code - 97 + 10] = false;
                break;
            }
        }
    }
    return false;
}

//returns draw flag
bool System::RunCycle() {
    uint16_t opcode = memory_[counter_] << 8 | memory_[counter_ + 1];
    uint8_t* x = registers_ + ((opcode & 0x0F00) >> 8);
    uint8_t* y = registers_ + ((opcode & 0x00F0) >> 4);
    uint8_t n = (opcode & 0xF);
    uint8_t nn = (opcode & 0xFF);
    uint16_t nnn = (opcode & 0xFFF);

    printf("%x - %x [reg: %x] [1: %s] [index: %x]\n", counter_, opcode, registers_[0], keys_[1] ? "down" : "up", index_);

    bool draw = false;
    if(opcode == 0x00E0) {
        std::fill(display_, display_ + (8 * 32), 0);
        draw = true;
    } else if (opcode == 0x00EE){
        counter_ = stack_[0];
        printf("a %x %x\n", stack_[0], stack_[1]);
        memcpy(stack_, stack_ + 1, 15);
        printf("b %x\n", stack_[0]);
        stack_[15] = 0;
        return draw;
    }

    bool increment = true;
    switch((opcode & 0xF000) >> 12){
        case 0x1:
            memcpy(stack_ + 1, stack_, 15);
            stack_[0] = counter_;

            counter_ = nnn;
            increment = false;
            break;
        case 0x2:
            memcpy(stack_ + 1, stack_, 15);
            stack_[0] = counter_;

            counter_ = nnn;
            increment = false;
            break;
        case 0x3:
            if(*x == nn)
                counter_ += 4;
            else
                counter_ += 2;
            increment = false;
            break;
        case 0x4:
            if(*x != nn)
                counter_ += 4;
            else
                counter_ += 2;
            increment = false;
            break;
        case 0x5:
            if(*x == *y)
                counter_ += 4;
            else
                counter_ += 2;
            increment = false;
            break;
        case 0x6:
            *x = nn;
            break;
        case 0x7:
            *x += nn;
            break;
        case 0x8:
            HandleMath(opcode & 0x000F, x, y);
            break;
        case 0x9:
            if(*x != *y)
                counter_ += 4;
            else
                counter_ += 2;
            increment = false;
            break;
        case 0xA:
            index_ = nnn;
            break;
        case 0xB:
            counter_ = nnn + registers_[0];
            increment = false;
            break;
        case 0xC:
            *x = (rand() % 256) & nn;
            break;
        case 0xD: {
            uint8_t flipped = 0;
            for (int py = 0; py < n; py++) {
                int shift = *x % 8;
                int index = (*y * 8 + py * 8 + (*x % 64) / 8) % 256;
                uint8_t old_px = display_[index];
                uint8_t old_overflow = display_[index + 1];
                display_[index] ^= memory_[index_ + py] >> shift;

                if((*x % 64) < 56)
                    display_[index + 1] ^= memory_[index_ + py] << (8 - shift);

                if ((old_px & (display_[index] ^ 0xFF)) > 0)
                    flipped = 1;
                else if ((old_overflow & (display_[index + 1] ^ 0xFF)) > 0)
                    flipped = 1;
            }

            registers_[15] = flipped;
            draw = true;
            break;
        }
        case 0xE:
            if(nn == 0x9E) {
                if (keys_[*x]) {
                    counter_ += 2;
                }
            } else {
                if (!keys_[*x]) {
                    counter_ += 2;
                }
            }
            break;
        case 0xF:
            HandleMisc(nn, x);
            increment = false;
            break;
    }

    if(increment)
        counter_ += 2;
    return draw;
}

void System::HandleMath(uint8_t type, uint8_t* x, const uint8_t* y){
    switch(type){
        case 0x0:
            *x = *y;
            break;
        case 0x1:
            *x |= *y;
            break;
        case 0x2:
            *x &= *y;
            break;
        case 0x3:
            *x ^= *y;
            break;
        case 0x4:
            if(*x + *y > 255)
                registers_[15] = 1;
            else
                registers_[15] = 0;
            *x += *y;
            break;
        case 0x5:
            if(*x >= *y)
                registers_[15] = 1;
            else
                registers_[15] = 0;
            *x -= *y;
            break;
        case 0x6:
            registers_[15] = *x & 0b1;
            *x >>= 1;
            break;
        case 0x7:
            if(*y >= *x)
                registers_[15] = 1;
            else
                registers_[15] = 0;
            *x = *y - *x;
            break;
        case 0xE:
            registers_[15] = (*x & 0b10000000) >> 7;
            *x <<= 1;
            break;
        default: break;
    }
}

void System::HandleMisc(uint8_t type, uint8_t* x){
    bool increment = true;
    switch(type){
        case 0x07:
            *x = delay_timer_;
            break;
        case 0x0A:
            if(key_flag == -2)
                key_flag = -1;
            if(key_flag == -1) {
                for (int i = 0; i < 16; i++) {
                    if (keys_[i] && !last_keys_[i]) {
                        key_flag = i;
                        return;
                    }
                }
            } else {
                for (int i = 0; i < 16; i++) {
                    if (!keys_[key_flag] && last_keys_[key_flag]) {
                        *x = i;
                        counter_ += 2;
                        return;
                    }
                }
            }
            increment = false;
            break;
        case 0x15:
            delay_timer_ = *x;
            break;
        case 0x18:
            sound_timer_ = *x;
            break;
        case 0x1E:
            index_ += *x;
            break;
        case 0x29:
            index_ = *x * 5 + 0x50;
            break;
        case 0x33: {
            int hundreds = *x / 100;
            int tens = (*x % 100) / 10;
            int ones = *x % 10;
            memory_[index_] = hundreds;
            memory_[index_ + 1] = tens;
            memory_[index_ + 2] = ones;
            break;
        }
        case 0x55:
            memcpy(registers_, memory_ + index_, *x + 1);
            break;
        case 0x65:
            memcpy(memory_ + index_, memory_, *x + 1);
            break;
        default: break;
    }

    if(increment)
        counter_ += 2;
}