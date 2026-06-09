#ifndef EPISODES_H
#define EPISODES_H

#include <iostream>
#include <string>
#include <cstdint>
#include "series.h"

using namespace std;

class Episodes {
private:
    Series series;
    uint32_t episodeNumber;
public:
    Episodes();

    Episodes(Series series, uint32_t episodeNumber);

    Series getSeries();
    void setSeries(Series series);

    uint32_t getEpisodeNumber();
    void setEpisodeNumber(uint32_t episodeNumber);
};

#endif