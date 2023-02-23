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
