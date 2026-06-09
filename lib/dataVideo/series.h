#ifndef SERIES_H
#define SERIES_H

#include <iostream>
#include <string>
#include <cstdint>
#include "video.h"


class Series : public Video {
private:
    uint32_t season;
    std::string title;
public:
    Series();

    Series(uint32_t id, std::string name, uint32_t length, std::string genre, uint8_t rating, uint32_t season, std::string title);

    uint32_t getSeason();
    void setSeason(uint32_t season);

    std::string getTitle();
    void setTitle(std::string title);
};

#endif