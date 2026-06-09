#ifndef VIDEO_H
#define VIDEO_H

#include <iostream>
#include <string>
#include <cstdint>

using namespace std;

class Video {
private:
    uint32_t id;
    string name;
    uint32_t length;
    string genre;
    uint8_t rating;
public:
    Video();

    Video(uint32_t id, string name, uint32_t length, string genre, uint8_t rating);

    uint32_t getId();
    void setId(uint32_t id);

    string getName();
    void setName(string name);

    uint32_t getLength();
    void setLength(uint32_t length);

    string getGenre();
    void setGenre(string genre);

    uint8_t getRating();
    void setRating(uint8_t rating);
};

#endif