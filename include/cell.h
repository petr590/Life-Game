#ifndef LIFEGAME_CELL_H
#define LIFEGAME_CELL_H

#include <SFML/Graphics.hpp>

namespace lifegame {

    using namespace sf;

	static const char
			CELL_ON = 0x1,
			CELL_OFF = 0x0,
			CELL_WILL_CHANGE = 0x2,
			CELL_SIZE = 16;

    struct Cell {
        public:
            char value;

            Cell() {}

            Cell(char value):
                value(value) {}

            bool isOn() const;

            void on();

            void off();

            void setWillChange();

            bool willChange() const;

            void change();

            void draw(RenderWindow&, int x, int y);

            bool intersectsWith(Vector2i startPoint, Vector2i endPoint, int x, int y);

        protected:
            int sign(Vector2i startPoint, Vector2i endPoint, int x, int y);
    };
}

#endif // CELL_H_INCLUDED
