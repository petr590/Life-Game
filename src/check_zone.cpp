#ifndef LIFEGAME_CHECK_ZONE_CPP
#define LIFEGAME_CHECK_ZONE_CPP

#include "check_zone.h"

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

    const CheckZone
            &CheckZone::QUAD = *new QuadCheckZone(),
            &CheckZone::RHOMB = *new RhombCheckZone();
}

#endif // LIFEGAME_CHECK_ZONE_CPP
