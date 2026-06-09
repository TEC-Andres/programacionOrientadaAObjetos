#ifndef MOVIE_H
#define MOVIE_H

#include <iostream>
#include <string>
#include <cstdint>
#include "video.h"

using namespace std;

class Movie {
private:
    Video video;
    string director;
public:
    Movie();

    Movie(Video video, string director);

    Video getVideo();
    void setVideo(Video video);

    string getDirector();
    void setDirector(string director);
};

#endif