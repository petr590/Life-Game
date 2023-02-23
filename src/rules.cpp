#ifndef LIFEGAME_RULES_CPP
#define LIFEGAME_RULES_CPP

#include "rules.h"

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
