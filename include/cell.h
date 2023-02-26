#ifndef LIFEGAME_CELL_H
#define LIFEGAME_CELL_H

#include <SFML/Graphics.hpp>

#ifdef DRAW_PARALLEL
#include <mutex>
#endif // DRAW_PARALLEL

namespace lifegame {

    using namespace sf;

    #ifdef DRAW_PARALLEL
    using std::mutex;
    #endif // DRAW_PARALLEL

	static const char
			CELL_ON = 0x1,
			CELL_OFF = 0x0,
			CELL_WILL_CHANGE = 0x2;

    static const int
            MIN_CELL_SIZE = 4,
            MAX_CELL_SIZE = 64,
            DEFAULT_CELL_SIZE = 16;

    extern int CELL_SIZE;

    struct Cell {
        public:
            static RectangleShape whiteCellShape, blackCellShape; // Для однопоточной отрисовки

            static RectangleShape createCellShape(Color);

            char value;

            inline Cell() {}

            inline Cell(char value):
                value(value) {}

            inline bool isOn() const {
                return value & CELL_ON;
            }

            inline void on() {
                value = CELL_ON;
            }

            inline void off() {
                value = CELL_OFF;
            }

            inline void setWillChange() {
                value |= CELL_WILL_CHANGE;
            }

            inline bool willChange() const {
                return (value & CELL_WILL_CHANGE) != 0;
            }

            inline void change() {
                value = (value ^ CELL_ON) & CELL_ON;
            }

            void draw(RenderWindow&, int x, int y) const;

            static void drawCell(RenderWindow&, int x, int y);

            #ifdef DRAW_PARALLEL
            static void drawCellSynchronized(RenderWindow&, int x, int y, mutex& mtx);
            #endif // DRAW_PARALLEL

            bool intersectsWith(Vector2i startPoint, Vector2i endPoint, int x, int y) const;

        protected:
            static int sign(Vector2i startPoint, Vector2i endPoint, int x, int y);
    };
}

#endif // CELL_H_INCLUDED
