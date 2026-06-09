#include "episodes.h"

Episodes::Episodes(){
    episodeNumber = 0;
}

Episodes::Episodes(Series series, uint32_t episodeNumber){
    this -> series = series;
    this -> episodeNumber = episodeNumber;
}

Series Episodes::getSeries(){
    return series;
}

void Episodes::setSeries(Series series){
    this -> series = series;
}

uint32_t Episodes::getEpisodeNumber(){
    return episodeNumber;
}

void Episodes::setEpisodeNumber(uint32_t episodeNumber){
    this -> episodeNumber = episodeNumber;
}