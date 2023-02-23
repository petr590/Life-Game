#ifndef LIFEGAME_RULES_H
#define LIFEGAME_RULES_H

#include <type_traits>
#include <string>
#include "rule.h"
#include "cell.h"

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
