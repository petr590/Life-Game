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
