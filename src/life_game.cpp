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
            dataWidth(widthOf(videoMode.width)), dataHeight(heightOf(videoMode.height)),
            width(dataWidth), height(dataHeight),
            data(new_2d_array<Cell>(width + 2, height + 2, 1, CELL_OFF)), // Резервируем область шириной в 1 клетку вокруг поля
            window(videoMode, TITLE, fullscreen ? Style::Fullscreen : Style::Default),
            fullscreen(fullscreen),

            defaultTextFont(loadFont(defaultFontName)),
            pausedText(defaultText(14)),
            rulesText(defaultText(27)),
            checkZoneText(defaultText(23)),
            speedText(defaultText(12)),
            scaleText(defaultText(10)),
            helpElement(Vector2f(window.getSize()), Vector2f(270.f / 16 * CELL_SIZE, 300.f / 16 * CELL_SIZE), {
                    defaultText(0, 0, "F1 - this help"),
                    defaultText(0, 0, "P - pause"),
                    defaultText(0, 0, "Esc - exit"),
                    defaultText(0, 0, "F11 - enter/exit fullscreen"),
                    defaultText(0, 0, "C - clear screen"),
                    defaultText(0, 0, "F - fill screen"),
                    defaultText(0, 0, "R - fill screen randomly"),
                    defaultText(0, 0, "1, 2, 3, 4 - change rules"),
                    defaultText(0, 0, "Z - change check zone"),
                    defaultText(0, 0, "LMB - draw"),
                    defaultText(0, 0, "RMB - erase"),
                    defaultText(0, 0, "Wheel up - increase speed"),
                    defaultText(0, 0, "Wheel down - reduce speed"),
            }) {

        setPause(true);
        setRules(&RULES[0]);
        setCheckZone(&CheckZone::QUAD);
        setDelay(64ms);
        setScale(DEFAULT_CELL_SIZE);
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

    Text LifeGame::defaultText(int x, int y, string content) {
        Text text(content, defaultTextFont, CHAR_WIDTH);
        text.setPosition(x, y);
        text.setFillColor(Color::White);

        return text;
    }

    Text LifeGame::defaultText(int charsWidth) {
        Text text = defaultText(textXOffset, height * CELL_SIZE - TOOLBAR_TEXT_OFFSET);
        textXOffset += charsWidth * CHAR_WIDTH / 2;
        return text;
    }


    LifeGame::HelpElement::HelpElement(Vector2f windowSize, Vector2f size, initializer_list<Text> texts):
            frame(size), texts(texts) {

        Vector2f position = (windowSize - size) / 2.f;

        frame.setPosition(position);
        frame.setFillColor(Color::Black);
        frame.setOutlineColor(Color::White);
        frame.setOutlineThickness(2);

        position.x += CELL_SIZE * 1.25f;
        position.y += CELL_SIZE;

        for(sf::Text& text : this->texts) {
            text.setPosition(position);
            position.y += CELL_SIZE * 1.25f;
        }
    }

    void LifeGame::HelpElement::hide() {
        hidden = true;
    }

    void LifeGame::HelpElement::toggleHidden() {
        hidden = !hidden;
    }

    bool LifeGame::HelpElement::isHidden() const {
        return hidden;
    }

    void LifeGame::HelpElement::draw(RenderTarget& target, RenderStates states) const {
        target.draw(frame);
        for(Text text : texts)
            target.draw(text);
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

    void LifeGame::setScale(int scale) {
        scale = min(max(scale, MIN_CELL_SIZE), MAX_CELL_SIZE);

        float multiplier = (float)scale / CELL_SIZE;

        width /= multiplier;
        height /= multiplier;
        extendDataIfNecessary();

        CELL_SIZE = scale;
        Vector2f newCellSize(CELL_SIZE - 1, CELL_SIZE - 1);
        Cell::whiteCellShape.setSize(newCellSize);
        Cell::blackCellShape.setSize(newCellSize);

        scaleText.setString("scale: " + fp_to_string((float)CELL_SIZE / DEFAULT_CELL_SIZE));
    }

    void LifeGame::incScale(int extent) {
        float multiplier = pow(2, extent);
        setScale(CELL_SIZE * multiplier);
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

    #ifdef DRAW_PARALLEL
    void LifeGame::forEachCellParallel(function<void(int, int, Cell&)> parallelFunc, function<void(int, int, Cell&)> func) {
        unsigned int processorsCount = thread::hardware_concurrency();

        cout << "processorsCount = " << processorsCount << endl;
        getchar();

        if(processorsCount > 1) {
            unsigned int threadsCount = processorsCount - 1;

            //vector<thread*> threads;
            vector<future<void>> futures;
            futures.reserve(threadsCount);

            int partWidth = width / processorsCount,
                height = this->height;

            for(unsigned int i = 0; i < threadsCount; ++i) {
                cout << "new thread" << endl;
                getchar();

                /*
                threads.push_back(new thread([this, i, partWidth, height, &parallelFunc] () {
                    forEachCell(i * partWidth, 0, (i + 1) * partWidth, height, parallelFunc);
                }));
                //*/

                futures.push_back(async(launch::async, [this, i, partWidth, height, &parallelFunc] () {
                    try {
                        forEachCell(i * partWidth, 0, (i + 1) * partWidth, height, parallelFunc);
                    } catch(std::exception& ex) {
                        cout << ex.what() << endl;
                        getchar();
                        throw;
                    }
                }));
            }

            cout << "threads started" << endl;
            getchar();

            forEachCell(threadsCount * partWidth, 0, processorsCount * partWidth, height, func);

            /*
            for(thread* thrd : threads) {
                //cout << "join thread" << endl;
                //getchar();

                thrd->join();
                delete thrd;
            }
            //*/

            for(future<void>& fut : futures) {
                fut.wait();
            }

            cout << "threads joined" << endl;
            getchar();

        } else {
            forEachCell(func);
        }
    }
    #endif // DRAW_PARALLEL

    void LifeGame::extendDataIfNecessary() {
        if(width > dataWidth || height > dataHeight) {
            Cell
                *const *const oldData = this->data,
                *const *const newData = new_2d_array<Cell>(width + 2, height + 2, 1, CELL_OFF);

            for(int x = 0, dataWidth = this->dataWidth; x < dataWidth; ++x) {
                Cell
                    *const oldRow = oldData[x],
                    *const newRow = newData[x];

                for(int y = 0, dataHeight = this->dataHeight; y < dataHeight; ++y) {
                    newRow[y] = oldRow[y];
                }
            }

            data = newData;
            dataWidth = width;
            dataHeight = height;
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
            cell = (rand() * x * y / 2) & CELL_ON;
        });
    }

    void LifeGame::clear() {
        forEachCell([] (int x, int y, Cell& cell) {
            cell.off();
        });
    }

    void LifeGame::fill() {
        forEachCell([] (int x, int y, Cell& cell) {
            cell.on();
        });
    }

    bool LifeGame::processEvent(Event& event) {
        switch(event.type) {
            case Event::Closed:
                window.close();
                return false;

            case Event::Resized:
                window.setView(View(FloatRect(0, 0, event.size.width, event.size.height)));
                width = widthOf(event.size.width);
                height = heightOf(event.size.height);

                for(Text* text : texts) {
                    text->setPosition(text->getPosition().x, event.size.height - TOOLBAR_TEXT_OFFSET);
                }

                break;

            case Event::KeyPressed:

                switch(event.key.code) {
                    case Keyboard::Escape:

                        if(!helpElement.isHidden()) {
                            helpElement.hide();
                        } else {
                            window.close();
                            return false;
                        }

                        break;

                    case Keyboard::F1:
                        helpElement.toggleHidden();
                        break;

                    case Keyboard::F2:
                        break;

                    case Keyboard::F11: {
                        Vector2u size = window.getSize();
                        window.create(VideoMode(size.x, size.y), TITLE, fullscreen ? Style::Default : Style::Fullscreen);
                        fullscreen = !fullscreen;
                        break;
                    }

                    case Keyboard::R:
                        fillRandom();
                        break;

                    case Keyboard::C:
                        clear();
                        break;

                    case Keyboard::F:
                        fill();
                        break;

                    case Keyboard::Space:
                        setPause(!paused);

                        if(!paused) { // reset time point
                            timePoint = system_clock::now();
                        }

                        break;

                    case Keyboard::Z:
                        setCheckZone(CheckZone::checkZones[checkZoneIndex = (checkZoneIndex + 1) % CheckZone::checkZones.size()]);
                        break;

                    case Keyboard::Up:
                        setDelay(delay / 2);
                        break;

                    case Keyboard::Down:
                        setDelay(delay * 2);
                        break;

                    default:

                        if(event.key.code >= Keyboard::Num1 && event.key.code <= Keyboard::Num9) {
                            setRules(&RULES[min((size_t)(event.key.code - Keyboard::Num1), RULES.size())]);
                            break;
                        }

                        return false;
                }

                break;

            case Event::MouseButtonPressed:
                if(event.mouseButton.button == Mouse::Left || event.mouseButton.button == Mouse::Right) {
                    userDrawingPos.x = event.mouseButton.x;
                    userDrawingPos.y = event.mouseButton.y;
                    userErasing = event.mouseButton.button == Mouse::Right;

                    int x = event.mouseButton.x / CELL_SIZE,
                        y = event.mouseButton.y / CELL_SIZE;

                    if(x >= 0 && x < width && y >= 0 && y < height) {
                        data[x][y] = userErasing ? CELL_OFF : CELL_ON;
                    }

                    break;
                }

                return false;

            case Event::MouseButtonReleased:
                if(event.mouseButton.button == Mouse::Left || event.mouseButton.button == Mouse::Right) {
                    userDrawingPos.x = userDrawingPos.y = -1;
                    userErasing = false;
                }

                return false;

            case Event::MouseMoved:
                if(userDrawingPos.x >= 0 && userDrawingPos.y >= 0) {

                    forEachCell(
                            min(userDrawingPos.x, event.mouseMove.x) / CELL_SIZE,     min(userDrawingPos.y, event.mouseMove.y) / CELL_SIZE,
                            max(userDrawingPos.x, event.mouseMove.x) / CELL_SIZE + 1, max(userDrawingPos.y, event.mouseMove.y) / CELL_SIZE + 1,
                            [this, &event] (int x, int y, Cell& cell) {
                                if(x >= 0 && x < width && y >= 0 && y < height &&
                                        cell.intersectsWith(userDrawingPos, Vector2i(event.mouseMove.x, event.mouseMove.y), x, y)) {

                                    cell = userErasing ? CELL_OFF : CELL_ON;
                                    cell.draw(window, x, y);
                                }
                            }
                    );

                    userDrawingPos.x = event.mouseMove.x;
                    userDrawingPos.y = event.mouseMove.y;

                    break;
                }

                return false;

            case Event::MouseWheelScrolled:
                if(event.mouseWheelScroll.wheel == Mouse::VerticalWheel) {
                    incScale(event.mouseWheelScroll.delta);
                    break;
                }

                return false;

            default:
                return false;
        }

        return true;
    }

    bool LifeGame::processEvents() {
        Event event;

        bool eventProcessed = false, needRedraw = false;

        while(window.pollEvent(event)) {

            needRedraw |= processEvent(event);
            eventProcessed = true;
        }

        if(needRedraw)
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
         #ifdef DEBUG
        static uint64_t tolalTicks = 0;
        static uint32_t tolalSteps = 0;
        #endif // DEBUG

        if(paused) {
            this_thread::sleep_for(MAX_SLEEP_TIME);
            processEvents();

             #ifdef DEBUG
            if(tolalTicks != 0) {
                uint64_t elapsed = tolalTicks / tolalSteps;
                cout << "average: " << elapsed << " (" << elapsed / (1024.f * 1024.f) << " M)" << endl;
                tolalTicks = 0;
                tolalSteps = 0;
            }
            #endif // DEBUG

        } else {
            #ifdef DEBUG
            {
                uint64_t start = rdtsc();
                step();
                uint64_t elapsed = rdtsc() - start;
                //cout << "elapsed: " << elapsed << " (" << elapsed / (1024.f * 1024.f) << " M)" << endl;
                tolalTicks += elapsed;
                ++tolalSteps;
            }
            #else
            step();
            #endif // DEBUG

            const auto nextTimePoint = this->timePoint += delay;

            while(true) {
                auto remainingTime = nextTimePoint - system_clock::now();

                bool remainedFew = remainingTime <= MAX_SLEEP_TIME;

                this_thread::sleep_for(remainedFew ? remainingTime : MAX_SLEEP_TIME);

                if(processEvents() && paused)
                    return;

                if(remainedFew)
                    break;
            }
        }
    }

    void LifeGame::drawAll() {
        window.clear();

        #ifdef DRAW_PARALLEL
        mutex mtx;

        forEachCellParallel(
                [this, &mtx] (int x, int y, Cell& cell) {
                    if(cell.isOn())
                        Cell::drawCellSynchronized(window, x, y, mtx);
                },

                [this] (int x, int y, Cell& cell) {
                    if(cell.isOn())
                        Cell::drawCell(window, x, y);
                }
        );
        #else
        forEachCell([this] (int x, int y, Cell& cell) {
            if(cell.isOn())
                Cell::drawCell(window, x, y);
        });
        #endif // DRAW_PARALLEL

        for(Text* text : texts) {
            window.draw(*text);
        }

        if(!helpElement.isHidden())
            window.draw(helpElement);

        #ifdef TRY_OPTIMIZE_RENDER
        // Из-за двойной буферизации OpenGL приходится вызывать window.display()
        // два раза, иначе отрисовывается предыдущий кадр
        window.display();
        window.display();
        #else
        window.display();
        #endif // TRY_OPTIMIZE_RENDER
    }

    void LifeGame::step() {
        //clearBorder();

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
                #ifdef TRY_OPTIMIZE_RENDER
                cell.draw(window, x, y);
                #endif // TRY_OPTIMIZE_RENDER
            }
        });

        #ifdef TRY_OPTIMIZE_RENDER
        window.display();
        #else
        drawAll();
        #endif // TRY_OPTIMIZE_RENDER
    }
}

#endif // LIFE_GAME_CPP
