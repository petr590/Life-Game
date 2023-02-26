#ifndef LIFEGAME_CHECK_ZONE_H
#define LIFEGAME_CHECK_ZONE_H

#include <string>
#include "cell.h"

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
