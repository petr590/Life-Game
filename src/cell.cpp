#ifndef LIFEGAME_CELL_CPP
#define LIFEGAME_CELL_CPP

#include "cell.h"

namespace lifegame {

    bool Cell::isOn() const {
        return value & CELL_ON;
    }

    void Cell::on() {
        value = CELL_ON;
    }

    void Cell::off() {
        value = CELL_OFF;
    }

    void Cell::setWillChange() {
        value |= CELL_WILL_CHANGE;
    }

    bool Cell::willChange() const {
        return (value & CELL_WILL_CHANGE) != 0;
    }

    void Cell::change() {
        value = (value ^ CELL_ON) & CELL_ON;
    }

    void Cell::draw(RenderWindow& window, int x, int y) {
        /*
        RectangleShape cellShape(Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));

        cellShape.setPosition(x * CELL_SIZE, y * CELL_SIZE);

        if(isOn()) {
            cellShape.setFillColor(Color::White);
        } else {
            cellShape.setFillColor(Color::Black);
        }

        cellShape.setOutlineThickness(1);
        cellShape.setOutlineColor(Color::Black);

        window.draw(cellShape);
        //*/

        if(isOn()) {
            RectangleShape cellShape(Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));

            cellShape.setPosition(x * CELL_SIZE, y * CELL_SIZE);
            cellShape.setFillColor(Color::White);

            window.draw(cellShape);
        }
    }

    bool Cell::intersectsWith(Vector2i startPoint, Vector2i endPoint, int x, int y) {
        x *= CELL_SIZE;
        y *= CELL_SIZE;

        int sig1 = sign(startPoint, endPoint, x, y);

        return sig1 != sign(startPoint, endPoint, x + (CELL_SIZE - 1), y) ||
               sig1 != sign(startPoint, endPoint, x, y + (CELL_SIZE - 1)) ||
               sig1 != sign(startPoint, endPoint, x + (CELL_SIZE - 1), y + (CELL_SIZE - 1));
    }

    int Cell::sign(Vector2i startPoint, Vector2i endPoint, int x, int y) {
        int value = (endPoint.y - startPoint.y) * (x - startPoint.x) - (endPoint.x - startPoint.x) * (y - startPoint.y);
        return (value > 0) - (value < 0);
    }
}

#endif
