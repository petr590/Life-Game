#include "life_game.h"
#include <iostream>

int main(int argc, const char* args[]) {
    using namespace lifegame;
    using std::cout;
    using std::cerr;
    using std::endl;
    using std::exception;

    srand(time(nullptr));

    /*
    int** array = new_2d_array<int>(12, 12, 1, 999);

    for(int x = -1; x < 11; ++x) {
        for(int y = -1; y < 11; ++y) {
            std::cout << array[x][y] << ' ';
        }

        std::cout << std::endl;
    }

    getchar();
    //*/

    //*
    try {
        LifeGame game(VideoMode::getFullscreenModes()[0]);
        game.fillRandom();
        game.run();

    } catch(exception& ex) {
        cerr << ex.what() << endl;
        return 1;
    }
    //*/

    return 0;
}
