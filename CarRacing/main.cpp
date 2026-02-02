#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "Game.h"

int main(int argc, char* argv[]) {
    CarRacing game = CarRacing();
    game.play();
    return 0;
}