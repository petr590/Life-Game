#ifndef LIFE_GAME_H
#define LIFE_GAME_H

#include <vector>
#include <functional>
#include <chrono>
#include <thread>

#ifdef DRAW_PARALLEL
#include <mutex>
#endif // DRAW_PARALLEL

#include <future>
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
    using std::initializer_list;
    using std::chrono::system_clock;
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    using std::chrono::operator"" ms;

    namespace this_thread = std::this_thread;

    #ifdef DRAW_PARALLEL
    using std::thread;
    using std::mutex;
    using std::future;
    using std::async;
    using std::launch;
    #endif // DRAW_PARALLEL

    static const char* const TITLE = "Life Game";

    class LifeGame {
            static const milliseconds MAX_SLEEP_TIME, MIN_DELAY, MAX_DELAY;
            static const vector<Rules> RULES;

            static const int
                    TOOLBAR_HEIGHT = 32,
                    CHAR_WIDTH = 16,
                    TOOLBAR_TEXT_OFFSET = 32 - CHAR_WIDTH / 2;

            int dataWidth, dataHeight;
            int width, height;
            Cell* const* data;

            RenderWindow window;
            bool fullscreen;

            bool paused;

            bool userErasing = false;
            Vector2i userDrawingPos{-1, -1};

            const Rules* rules;
            const CheckZone* checkZone;
            unsigned int checkZoneIndex = 0;

            system_clock::time_point timePoint = system_clock::now();
            milliseconds delay;

            Font defaultTextFont;
            int textXOffset = CHAR_WIDTH;
            Text pausedText, rulesText, checkZoneText, speedText, scaleText;
            vector<Text*> texts = { &pausedText, &rulesText, &checkZoneText, &speedText, &scaleText };


            class HelpElement: public Drawable {
                private:
                    bool hidden = true;
                    RectangleShape frame;
                    vector<Text> texts;

                public:
                    HelpElement(Vector2f windowSize, Vector2f size, initializer_list<Text> texts);

                    void hide();

                    void toggleHidden();

                    bool isHidden() const;

                protected:
                    virtual void draw(RenderTarget&, RenderStates) const override;

            } helpElement;


            Text defaultText(int x, int y, string content = "");
            Text defaultText(int charsWidth);

            int widthOf(int width);
            int heightOf(int width);

        public:
            void setPause(bool paused);
            void setRules(const Rules*);
            void setCheckZone(const CheckZone*);
            void setDelay(milliseconds delay);
            void setScale(int scale);

            void incScale(int extent);

            LifeGame(VideoMode, bool fullscreen = false, string defaultFontName = "sans-serif.ttf");
            ~LifeGame();

        protected:
            void extendDataIfNecessary();

            void forEachCell(function<void(int, int, Cell&)>);

            void forEachCell(int x, int y, int endX, int endY, function<void(int, int, Cell&)>);

            #ifdef DRAW_PARALLEL
            void forEachCellParallel(function<void(int, int, Cell&)> parallelFunc, function<void(int, int, Cell&)> func);
            #endif // DRAW_PARALLEL

            void clearBorder();

        public:
            void fillRandom();

            void clear();

            void fill();

        protected:
            bool processEvent(Event&);

        public:
            bool processEvents();

            void run();

        protected:
            void iteration();

            void drawAll();

        public:
            void step();
    };
}

#endif // LIFE_GAME_H
