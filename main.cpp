#include <iostream>
#include <libc.h>
#include "System.h"

int main() {
    System system("games/chip8-test-suite.ch8");
    system.Start();
    return 0;
}
