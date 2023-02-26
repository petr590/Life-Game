#include "life_game.h"
#include <iostream>

int main(int argc, const char* args[]) {
    using namespace lifegame;
    using std::cout;
    using std::cerr;
    using std::endl;
    using std::exception;

    srand(time(nullptr));

    try {
        LifeGame game(VideoMode::getFullscreenModes()[0]);

        #if 1
        game.fillRandom();
        game.run();
        #else

        game.incScale(-2);
        game.fillRandom();
        game.setDelay(16ms);

        uint64_t tolalTicks = 0;
        uint32_t tolalSteps = 0;

        for(int i = 0; i < 100; ++i) {
            uint64_t start = rdtsc();

            game.step();

            uint64_t elapsed = rdtsc() - start;
            //cout << "elapsed: " << elapsed << " (" << elapsed / (1024.f * 1024.f) << " M)" << endl;
            tolalTicks += elapsed;
            ++tolalSteps;
        }

        uint64_t average = tolalTicks / tolalSteps;
        cout << "average: " << average << " (" << average / (1024.f * 1024.f) << " M)" << endl;
        getchar();

        #endif // 0

    } catch(exception& ex) {
        cerr << ex.what() << endl;
        return 1;
    }

    return 0;
}
