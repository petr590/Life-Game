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
#ifndef LIFEGAME_CHECK_ZONE_H
#define LIFEGAME_CHECK_ZONE_H

#include <string>

namespace lifegame {

    using std::string;
    using std::vector;

    class CheckZone {
        public:
            static const CheckZone &QUAD, &RHOMB, &CROSS;
            static const vector<const CheckZone*> checkZones;

            const string name;

            CheckZone(string name):
                    name(name) {}

            virtual ~CheckZone() {}

            virtual int countNeighbours(Cell* prevRow, Cell* currRow, Cell* nextRow) const = 0;
    };

    class QuadCheckZone: public CheckZone {
        public:
            QuadCheckZone(): CheckZone("quad") {}

            virtual int countNeighbours(Cell* prevRow, Cell* currRow, Cell* nextRow) const override;
    };

    class RhombCheckZone: public CheckZone {
        public:
            RhombCheckZone(): CheckZone("rhomb") {}

            virtual int countNeighbours(Cell* prevRow, Cell* currRow, Cell* nextRow) const override;
    };

    class CrossCheckZone: public CheckZone {
        public:
            CrossCheckZone(): CheckZone("cross") {}

            virtual int countNeighbours(Cell* prevRow, Cell* currRow, Cell* nextRow) const override;
    };
}

#endif // LIFEGAME_CHECK_ZONE_H
#ifndef LIFEGAME_FONT_LOAD_EXCEPTION_H
#define LIFEGAME_FONT_LOAD_EXCEPTION_H

#include <stdexcept>

namespace lifegame {

    using std::runtime_error;
    using std::string;

    class FontLoadException: public runtime_error {
        public:
            FontLoadException(const string& what);
            FontLoadException(const char* what);
    };
}

#endif // LIFEGAME_FONT_LOAD_EXCEPTION_H
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

    using clock = std::chrono::steady_clock;
    using duration   = clock::duration;
    using time_point = clock::time_point;

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
            static const duration MAX_SLEEP_TIME, MIN_DELAY, MAX_DELAY, MIN_RENDER_DELAY;
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

            time_point
                    timePoint = clock::now(),
                    renderTimePoint = timePoint;

            duration delay;

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
            void setDelay(duration delay);
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
#ifndef LIFEGAME_RULE_H
#define LIFEGAME_RULE_H

#include <type_traits>

namespace lifegame {

    typedef int rule_t;

    static const int MAX_RULE_NUMS = 8;

    struct Rule {

        const rule_t value;

        constexpr Rule(rule_t value):
            value(value) {}

        bool matches(int neighbors) const;
    };


	template<typename V, typename... W>
	static constexpr rule_t makeRule(V v, W... ws) {
		static_assert(std::is_same<V, rule_t>(), "Only rule_t allowed");

		if constexpr(sizeof...(W) == 0) {
			return 1 << v;
		} else {
			return 1 << v | makeRule(ws...);
		}
	}
}

#endif // LIFEGAME_RULE_H
#ifndef LIFEGAME_RULES_H
#define LIFEGAME_RULES_H

#include <type_traits>
#include <string>

namespace lifegame {

    using std::string;

    class Rules {
        public:
        const Rule birth, survive;
        const string name;

        private:
        static void writeName(string& name, Rule);
        static string nameFor(Rule birth, Rule survive);

        public:
        Rules(Rule birth, Rule survive);

        bool matches(Cell, int neighbours) const;
    };
}

#endif // LIFEGAME_RULES_H
#ifndef LIFEGAME_UTIL_H
#define LIFEGAME_UTIL_H

#include <cstddef>
#include <string>
#include <sstream>
#include <iostream>
#include <SFML/Graphics.hpp>

namespace lifegame {

    using std::cout;
    using std::cerr;
    using std::endl;
    using std::string;
    using std::stringstream;
    using std::size_t;
    using namespace sf;

    /**
     * Выделяет двумерный массив указанных размеров одним блоком
     * Для удаления нужно просто вызвать delete[] array;
     */
    template<typename T>
    static T** new_2d_array(size_t width, size_t height, size_t offset = 0) {
        T** array = static_cast<T**>(static_cast<void*>(new char[width * sizeof(T*) + width * height * sizeof(T)]));

        T* const data = static_cast<T*>(static_cast<void*>(array + width));
        for(size_t i = 0; i < width; ++i) {
            array[i] = data + i * height + offset;
        }

        return array + offset;
    }

    template<typename T>
    static T** new_2d_array(size_t width, size_t height, size_t offset, T fillValue) {
        T** array = new_2d_array<T>(width, height, offset);

        for(T *data = static_cast<T*>(static_cast<void*>(array - offset + width)), *end = data + width * height; data < end; ++data) {
            *data = fillValue;
        }

        return array;
    }

    Font loadFont(string name);

    string fp_to_string(float num);

    #ifdef DEBUG
    static uint64_t rdtsc() {
        unsigned int low, high;
        asm volatile ("rdtsc" : "=a" (low), "=d" (high));
        return ((uint64_t)high << 32) | low;
    }
    #endif // DEBUG
}

#endif // LIFEGAME_UTIL_H
#ifndef LIFEGAME_CELL_CPP
#define LIFEGAME_CELL_CPP


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
#ifndef LIFEGAME_CHECK_ZONE_CPP
#define LIFEGAME_CHECK_ZONE_CPP


namespace lifegame {

    int QuadCheckZone::countNeighbours(Cell* prevRow, Cell* currRow, Cell* nextRow) const {
        return prevRow[-1].isOn() + prevRow[0].isOn() + prevRow[1].isOn() +
               currRow[-1].isOn() +                     currRow[1].isOn() +
               nextRow[-1].isOn() + nextRow[0].isOn() + nextRow[1].isOn();
    }

    int RhombCheckZone::countNeighbours(Cell* prevRow, Cell* currRow, Cell* nextRow) const {
        return                      prevRow[0].isOn() +
               currRow[-1].isOn() +                     currRow[1].isOn() +
                                    nextRow[0].isOn();
    }

    int CrossCheckZone::countNeighbours(Cell* prevRow, Cell* currRow, Cell* nextRow) const {
        return prevRow[-1].isOn() + prevRow[1].isOn() +
               nextRow[-1].isOn() + nextRow[1].isOn();
    }

    const CheckZone
            &CheckZone::QUAD = *new QuadCheckZone(),
            &CheckZone::RHOMB = *new RhombCheckZone(),
            &CheckZone::CROSS = *new CrossCheckZone();

    const vector<const CheckZone*> CheckZone::checkZones { &CheckZone::QUAD, &CheckZone::RHOMB, &CheckZone::CROSS };
}

#endif // LIFEGAME_CHECK_ZONE_CPP
#ifndef LIFEGAME_FONT_LOAD_EXCEPTION_CPP
#define LIFEGAME_FONT_LOAD_EXCEPTION_CPP


namespace lifegame {

    FontLoadException::FontLoadException(const string& what):
            runtime_error(what) {}

    FontLoadException::FontLoadException(const char* what):
            runtime_error(what) {}
}

#endif // LIFEGAME_FONT_LOAD_EXCEPTION_CPP
#ifndef LIFE_GAME_CPP
#define LIFE_GAME_CPP


namespace lifegame {

    const duration
            LifeGame::MAX_SLEEP_TIME = 100ms,
            LifeGame::MIN_DELAY        = 0x004ms,
            LifeGame::MAX_DELAY        = 0x400ms,
            LifeGame::MIN_RENDER_DELAY = 0x020ms;

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

    void LifeGame::setDelay(duration delay) {
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
                            timePoint = clock::now();
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

            if(timePoint >= renderTimePoint) {
                renderTimePoint += max(delay, MIN_RENDER_DELAY);
                #ifdef TRY_OPTIMIZE_RENDER
                window.display();
                #else
                drawAll();
                #endif // TRY_OPTIMIZE_RENDER
            }

            const time_point nextTimePoint = timePoint += delay;

            //cout << nextTimePoint.time_since_epoch().count() << endl;

            while(true) {
                const duration remainingTime = nextTimePoint - clock::now();

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
    }
}

#endif // LIFE_GAME_CPP
#ifndef LIFEGAME_RULE_CPP
#define LIFEGAME_RULE_CPP


namespace lifegame {

    bool Rule::matches(int neighbors) const {
        return (value & (1 << neighbors)) != 0;
    }
}

#endif // LIFEGAME_RULE_CPP
#ifndef LIFEGAME_RULES_CPP
#define LIFEGAME_RULES_CPP


namespace lifegame {

    Rules::Rules(Rule birth, Rule survive):
            birth(birth), survive(survive), name(nameFor(birth, survive)) {}


    void Rules::writeName(string& name, Rule rule) {
        for(int neighbors = 0; neighbors <= MAX_RULE_NUMS; ++neighbors) {
            if(rule.matches(neighbors))
                name += (char)('0' + neighbors);
        }
    }

    string Rules::nameFor(Rule birth, Rule survive) {
        string name;
        name.reserve((MAX_RULE_NUMS + 1) * 2 + 5);

        name += "B:";
        writeName(name, birth);
        name += " S:";
        writeName(name, survive);

        return name;
    }

    bool Rules::matches(Cell cell, int neighbours) const {
        return cell.isOn() ? !survive.matches(neighbours) : birth.matches(neighbours);
    }
}

#endif // LIFEGAME_RULES_CPP
#ifndef LIFEGAME_UTIL_CPP
#define LIFEGAME_UTIL_CPP


namespace lifegame {

    Font loadFont(string name) {
        Font font;

        if(!font.loadFromFile(name)) {
            cerr << "Cannot loading font" << endl;
            throw FontLoadException("Cannot load font \"" + name + "\"");
        }

        return font;
    }

    string fp_to_string(float num) {
        stringstream ss;
        ss << num;
        return ss.str();
    }
}

#endif // LIFEGAME_UTIL_CPP

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
