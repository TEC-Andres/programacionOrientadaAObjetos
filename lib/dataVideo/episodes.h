#ifndef EPISODES_H
#define EPISODES_H

#include <iostream>
#include <string>
#include <cstdint>
#include "series.h"

class Episodes : public Series {
private:
    uint32_t episodeNumber;
public:
    Episodes();
    Episodes(Video &v, std::string director, uint32_t season, std::string title, uint32_t episodeNumber);
    Episodes(uint32_t id, std::string name, uint32_t length, std::string genre, uint8_t rating, uint32_t season, std::string title, uint32_t episodeNumber);

    uint32_t getEpisodeNumber();
    void setEpisodeNumber(uint32_t episodeNumber);
};

#endif