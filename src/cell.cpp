#ifndef LIFEGAME_CELL_CPP
#define LIFEGAME_CELL_CPP

#include "cell.h"

namespace lifegame {

    int CELL_SIZE = DEFAULT_CELL_SIZE;

    RectangleShape
            Cell::whiteCellShape = Cell::createCellShape(Color::White),
            Cell::blackCellShape = Cell::createCellShape(Color::Black);

    RectangleShape Cell::createCellShape(Color color) {
        RectangleShape cellShape(Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
        cellShape.setFillColor(color);
        return cellShape;
    }


    void Cell::draw(RenderWindow& window, int x, int y) const {
        RectangleShape& cellShape = isOn() ? whiteCellShape : blackCellShape;
        cellShape.setPosition(x * CELL_SIZE, y * CELL_SIZE);
        window.draw(cellShape);
    }

    void Cell::drawCell(RenderWindow& window, int x, int y) {
        whiteCellShape.setPosition(x * CELL_SIZE, y * CELL_SIZE);
        window.draw(whiteCellShape);
    }

    #ifdef DRAW_PARALLEL
    void Cell::drawCellSynchronized(RenderWindow& window, int x, int y, mutex& mtx) {

        RectangleShape cellShape(Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));

        cellShape.setPosition(x * CELL_SIZE, y * CELL_SIZE);
        cellShape.setFillColor(Color::White);

        mtx.lock();
        window.draw(cellShape);
        mtx.unlock();
    }
    #endif // DRAW_PARALLEL

    bool Cell::intersectsWith(Vector2i startPoint, Vector2i endPoint, int x, int y) const {
        x *= CELL_SIZE;
        y *= CELL_SIZE;

        int sig1 = sign(startPoint, endPoint, x, y);

        return sig1 != sign(startPoint, endPoint, x + CELL_SIZE, y) ||
               sig1 != sign(startPoint, endPoint, x, y + CELL_SIZE) ||
               sig1 != sign(startPoint, endPoint, x + CELL_SIZE, y + CELL_SIZE);
    }

    int Cell::sign(Vector2i startPoint, Vector2i endPoint, int x, int y) {
        int value = (endPoint.y - startPoint.y) * (x - startPoint.x) - (endPoint.x - startPoint.x) * (y - startPoint.y);
        return (value > 0) - (value < 0);
    }
}

#endif
