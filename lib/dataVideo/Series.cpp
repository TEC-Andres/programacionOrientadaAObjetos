#include "series.h"

Series::Series() {
    season = 0;
    title = "";
}

Series::Series(Video video, uint32_t season, string title){
    this -> video = video;
    this -> season = season;
    this -> title = title;
}

Video Series::getVideo() {
    return video;
}

void Series::setVideo(Video video){
    this -> video = video;
}

uint32_t Series::getSeason(){
    return season;
}

void Series::setSeason(uint32_t season){
    this -> season = season;
}

string Series::getTitle() {
    return title;
}

void Series::setTitle(string title){
    this -> title = title;
}