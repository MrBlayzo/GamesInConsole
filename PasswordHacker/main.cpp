#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "Game.h"

int main(int argc, char* argv[]) {
    PasswordHacker game = PasswordHacker();
    game.play();
    return 0;
}