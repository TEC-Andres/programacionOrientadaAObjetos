#ifndef MOVIE_H
#define MOVIE_H

#include <iostream>
#include <string>
#include <cstdint>
#include "video.h"

class Movie : public Video {
private:
    std::string director;
public:
    Movie();

    Movie(uint32_t id, std::string name, uint32_t length, std::string genre, uint8_t rating, std::string director);

    std::string getDirector();
    void setDirector(std::string director);
};

#endif