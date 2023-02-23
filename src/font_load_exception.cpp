#ifndef LIFEGAME_FONT_LOAD_EXCEPTION_CPP
#define LIFEGAME_FONT_LOAD_EXCEPTION_CPP

#include "font_load_exception.h"

namespace lifegame {

    FontLoadException::FontLoadException(const string& what):
            runtime_error(what) {}

    FontLoadException::FontLoadException(const char* what):
            runtime_error(what) {}
}

#endif // LIFEGAME_FONT_LOAD_EXCEPTION_CPP
