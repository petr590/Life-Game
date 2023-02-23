#ifndef LIFE_GAME_H
#define LIFE_GAME_H

#include <vector>
#include <functional>
#include <chrono>
#include <thread>
#include <iostream>
#include <cmath>
#include "cell.h"
#include "rules.h"
#include "check_zone.h"
#include "util.h"

namespace lifegame {

    using std::cout;
    using std::cerr;
    using std::endl;
    using std::min;
    using std::max;
    using std::pow;
    using std::vector;
    using std::function;
    using std::to_string;
    using std::chrono::system_clock;
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    using std::chrono::operator"" ms;
    namespace this_thread = std::this_thread;

    static const char* const TITLE = "Life Game";

    class LifeGame {

            static const milliseconds MAX_SLEEP_TIME, MIN_DELAY, MAX_DELAY;
            static const vector<Rules> RULES;

            static const int
                    TOOLBAR_HEIGHT = 32,
                    CHAR_WIDTH = 16,
                    TOOLBAR_TEXT_OFFSET = 32 - CHAR_WIDTH / 2;


            int width, height;
            Cell* const* const data;
            RenderWindow window;
            bool fullscreen;

            bool paused;

            Vector2i userDrawingPos{-1, -1};
            bool userErasing = false;

            const Rules* rules;
            const CheckZone* checkZone;

            system_clock::time_point timePoint = system_clock::now();
            milliseconds delay;

            Font defaultTextFont;
            int textXOffset = CHAR_WIDTH;
            Text pausedText, rulesText, checkZoneText, speedText;
            vector<Text*> texts = { &pausedText, &rulesText, &checkZoneText, &speedText };

            Text defaultText(int charsWidth);

            int widthOf(int width);
            int heightOf(int width);

            void clearBorder();

        public:
            void setPause(bool paused);
            void setRules(const Rules*);
            void setCheckZone(const CheckZone*);
            void setDelay(milliseconds delay);

            void incDelay(int steps);

            LifeGame(VideoMode, bool fullscreen = false, string defaultFontName = "sans-serif.ttf");
            ~LifeGame();

        protected:
            void forEachCell(function<void(int, int, Cell&)>);

            void forEachCell(int x, int y, int endX, int endY, function<void(int, int, Cell&)>);

        public:
            void fillRandom();

            void clear();

            bool processEvent();

            void run();

        protected:
            void iteration();

            void drawAll();

            void step();
    };
}

#endif // LIFE_GAME_H
