#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include "Game.h"

int main(int argc, char *argv[])
{
    CarRacing game = CarRacing();
    game.play();
    return 0;
}