#ifndef SERIES_H
#define SERIES_H

#include <iostream>
#include <string>
#include <cstdint>
#include "video.h"

using namespace std;

class Series {
private:
    Video video;
    uint32_t season;
    string title;
public:
    Series();

    Series(Video video, uint32_t season, string title);

    Video getVideo();
    void setVideo(Video video);

    uint32_t getSeason();
    void setSeason(uint32_t season);

    string getTitle();
    void setTitle(string title);
};

#endif