#ifndef LIFE_GAME_CPP
#define LIFE_GAME_CPP

#include "life_game.h"

namespace lifegame {

    const milliseconds
            LifeGame::MAX_SLEEP_TIME = 100ms,
            LifeGame::MIN_DELAY = 0x004ms,
            LifeGame::MAX_DELAY = 0x400ms;

    const vector<Rules> LifeGame::RULES = {
        { makeRule(3),          makeRule(2, 3) },
        { makeRule(5, 6, 7, 8), makeRule(0, 1, 2, 3, 4, 5, 6, 7, 8) },
        { makeRule(5, 6, 7, 8), makeRule(4, 5, 6, 7, 8) },
        { makeRule(1),          makeRule(0, 1, 2, 3, 4, 5, 6, 7, 8) }
    };

    LifeGame::LifeGame(VideoMode videoMode, bool fullscreen, string defaultFontName):
            width(widthOf(videoMode.width)), height(heightOf(videoMode.height)),
            data(new_2d_array<Cell>(width + 2, height + 2, 1, CELL_OFF)), // Резервируем область шириной в 1 клетку вокруг поля
            window(videoMode, TITLE, fullscreen ? Style::Fullscreen : Style::Default),
            fullscreen(fullscreen),

            defaultTextFont(loadFont(defaultFontName)),
            pausedText(defaultText(14)),
            rulesText(defaultText(27)),
            checkZoneText(defaultText(23)),
            speedText(defaultText(10)) {

        setPause(true);
        setRules(&RULES[0]);
        setCheckZone(&CheckZone::QUAD);
        setDelay(64ms);
    }

    LifeGame::~LifeGame() {
        delete[] (data - 1);
    }

    int LifeGame::widthOf(int width) {
        return width / CELL_SIZE;
    }

    int LifeGame::heightOf(int height) {
        return (height - TOOLBAR_HEIGHT) / CELL_SIZE;
    }

    Text LifeGame::defaultText(int charsWidth) {
        Text text("", defaultTextFont, CHAR_WIDTH);
        text.setPosition(textXOffset, height * CELL_SIZE - TOOLBAR_TEXT_OFFSET);
        text.setFillColor(Color::White);

        textXOffset += charsWidth * CHAR_WIDTH / 2;

        return text;
    }

    void LifeGame::setPause(bool paused) {
        this->paused = paused;
        pausedText.setString(paused ? "paused" : "not paused");
    }

    void LifeGame::setRules(const Rules* rules) {
        this->rules = rules;
        rulesText.setString("rules: " + rules->name);
    }

    void LifeGame::setCheckZone(const CheckZone* checkZone) {
        this->checkZone = checkZone;
        checkZoneText.setString("check zone: " + checkZone->name);
    }

    void LifeGame::setDelay(milliseconds delay) {
        this->delay = min(max(delay, MIN_DELAY), MAX_DELAY);
        speedText.setString("speed: " + to_string(MAX_DELAY.count() / this->delay.count()));
    }

    void LifeGame::incDelay(int steps) {
        setDelay(duration_cast<milliseconds>(delay * pow(2, steps)));
    }


    void LifeGame::forEachCell(function<void(int, int, Cell&)> func) {
        forEachCell(0, 0, width, height, func);
    }

    void LifeGame::forEachCell(int startX, int startY, int endX, int endY, function<void(int, int, Cell&)> func) {
        auto data = this->data;

        for(int x = startX; x < endX; ++x) {
            Cell* const row = data[x];

            for(int y = startY; y < endY; ++y) {
                func(x, y, row[y]);
            }
        }
    }

    void LifeGame::clearBorder() {
        int endX = width + 1,
            endY = height + 1;

        for(int x = -1; x < endX; ++x) {
            Cell *const border = data[x];

            border[-1].off();
            border[endY].off();
        }

        Cell *const leftBorder = data[-1],
             *const rightBorder = data[width];

        for(int y = 0, height = this->height; y < height; ++y) {
            leftBorder[y].off();
            rightBorder[y].off();
        }
    }

    void LifeGame::fillRandom() {
        forEachCell([] (int x, int y, Cell& cell) {
            cell = rand() * x * y / 2 & CELL_ON;
        });
    }

    void LifeGame::clear() {
        forEachCell([] (int x, int y, Cell& cell) {
            cell.off();
        });
    }

    bool LifeGame::processEvent() {
        Event event;

        bool eventProcessed = false;

        while(window.pollEvent(event)) {

            switch(event.type) {
                case Event::Closed:
                    window.close();
                    break;

                case Event::Resized:
                    window.setView(View(FloatRect(0, 0, event.size.width, event.size.height)));
                    width = widthOf(event.size.width);
                    height = heightOf(event.size.height);

                    for(sf::Text* text : texts) {
                        text->setPosition(text->getPosition().x, event.size.height - TOOLBAR_TEXT_OFFSET);
                    }

                    drawAll();

                    break;

                case Event::KeyPressed:

                    switch(event.key.code) {
                        case Keyboard::Escape:
                            window.close();
                            break;

                        case Keyboard::F11: {
                            Vector2u size = window.getSize();
                            window.create(VideoMode(size.x, size.y), TITLE, fullscreen ? Style::Default : Style::Fullscreen);
                            drawAll();
                            fullscreen = !fullscreen;
                            break;
                        }

                        case Keyboard::R:
                            fillRandom();
                            break;

                        case Keyboard::C:
                            clear();
                            break;

                        case Keyboard::Space:
                            setPause(!paused);

                            if(!paused) { // reset time point
                                timePoint = system_clock::now();
                            }

                            break;

                        case Keyboard::Z:
                            setCheckZone(checkZone == &CheckZone::QUAD ? &CheckZone::RHOMB : &CheckZone::QUAD);
                            break;

                        case Keyboard::Dash:
                            setDelay(delay * 2);
                            break;

                        case Keyboard::Equal:
                            setDelay(delay / 2);
                            break;

                        default:

                            if(event.key.code >= Keyboard::Num1 && event.key.code <= Keyboard::Num9) {
                                setRules(&RULES[min((size_t)(event.key.code - Keyboard::Num1), RULES.size())]);
                            }

                            break;
                    }

                    break;

                case Event::MouseButtonPressed:
                    if(event.mouseButton.button == Mouse::Left || event.mouseButton.button == Mouse::Right) {
                        userDrawingPos.x = event.mouseButton.x;
                        userDrawingPos.y = event.mouseButton.y;
                        userErasing = event.mouseButton.button == Mouse::Right;
                    }

                    break;

                case Event::MouseButtonReleased:
                    if(event.mouseButton.button == Mouse::Left) {
                        userDrawingPos.x = userDrawingPos.y = -1;
                        userErasing = false;
                    }

                    break;

                case Event::MouseMoved:
                    if(userDrawingPos.x >= 0 && userDrawingPos.y >= 0) {

                        forEachCell(
                                min(userDrawingPos.x, event.mouseMove.x) / CELL_SIZE,     min(userDrawingPos.y, event.mouseMove.y) / CELL_SIZE,
                                max(userDrawingPos.x, event.mouseMove.x) / CELL_SIZE + 1, max(userDrawingPos.y, event.mouseMove.y) / CELL_SIZE + 1,
                                [this, &event] (int x, int y, Cell& cell) {
                                    if(x >= 0 && x < width && y >= 0 && y < height &&
                                            cell.intersectsWith(userDrawingPos, Vector2i(event.mouseMove.x, event.mouseMove.y), x, y)) {

                                        cell = userErasing ? CELL_OFF : CELL_ON;
                                    }
                                }
                        );

                        userDrawingPos.x = event.mouseMove.x;
                        userDrawingPos.y = event.mouseMove.y;
                    }

                    break;

                case Event::MouseWheelScrolled:
                    if(event.mouseWheelScroll.wheel == Mouse::VerticalWheel) {
                        incDelay(-event.mouseWheelScroll.delta);
                    }

                    break;
            }

            eventProcessed = true;
        }

        if(eventProcessed)
            drawAll();

        return eventProcessed;
    }

    void LifeGame::run() {
        drawAll();

        while(window.isOpen()) {
            iteration();
        }
    }

    void LifeGame::iteration() {
        if(paused) {
            this_thread::sleep_for(MAX_SLEEP_TIME);
            processEvent();

        } else {
            step();
            const auto nextTimePoint = this->timePoint += delay;

            while(true) {
                auto remainingTime = nextTimePoint - system_clock::now();

                bool remainedFew = remainingTime <= MAX_SLEEP_TIME;

                this_thread::sleep_for(remainedFew ? remainingTime : MAX_SLEEP_TIME);

                if(processEvent() && paused)
                    return;

                if(remainedFew)
                    break;
            }
        }
    }

    void LifeGame::drawAll() {
        window.clear();

        forEachCell([this] (int x, int y, Cell& cell) {
            cell.draw(window, x, y);
        });

        for(Text* text : texts)
            window.draw(*text);

        window.display();
    }

    void LifeGame::step() {
        clearBorder();

        Cell* const* rowPtr = data;

        for(int x = 1, width = this->width; x <= width; ++x, ++rowPtr) {
            Cell
                *prevRow = rowPtr[-1],
                *currRow = rowPtr[0],
                *nextRow = rowPtr[1];

            for(int y = 0; y < height; ++y, ++prevRow, ++currRow, ++nextRow) {
                Cell& cell = currRow[0];

                if(rules->matches(cell, checkZone->countNeighbours(prevRow, currRow, nextRow))) {
                    cell.setWillChange();
                }
            }
        }

        forEachCell([this] (int x, int y, Cell& cell) {
            if(cell.willChange()) {
                cell.change();
                //cell.draw(window, x, y);
            }
        });

        drawAll();

        //window.display();
    }
}

#endif // LIFE_GAME_CPP
