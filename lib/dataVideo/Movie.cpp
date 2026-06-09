#include "movie.h"

Movie::Movie(){
    director = "";
}

Movie::Movie(Video video, string director) {
    this -> video = video;
    this -> director = director;
}

Video Movie::getVideo() {
    return video;
}

void Movie::setVideo(Video video) {
    this -> video = video;
}

string Movie::getDirector() {
    return director;
}

void Movie::setDirector(string director) {
    this -> director = director;
}