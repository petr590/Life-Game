#ifndef LIFEGAME_RULE_CPP
#define LIFEGAME_RULE_CPP

#include "rule.h"

namespace lifegame {

    bool Rule::matches(int neighbors) const {
        return (value & (1 << neighbors)) != 0;
    }
}

#endif // LIFEGAME_RULE_CPP
