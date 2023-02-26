#ifndef LIFEGAME_UTIL_CPP
#define LIFEGAME_UTIL_CPP

#include "util.h"

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
