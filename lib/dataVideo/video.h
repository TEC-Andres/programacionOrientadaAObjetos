#ifndef VIDEO_H
#define VIDEO_H

#include <iostream>
#include <string>
#include <cstdint>

class Video {
private:
    uint32_t id;
    std::string name;
    uint32_t length;
    std::string genre;
    uint8_t rating;
public:
    Video();

    Video(uint32_t id, std::string name, uint32_t length, std::string genre, uint8_t rating);

    uint32_t getId();
    void setId(uint32_t id);

    std::string getName();
    void setName(std::string name);

    uint32_t getLength();
    void setLength(uint32_t length);

    std::string getGenre();
    void setGenre(std::string genre);

    uint8_t getRating();
    void setRating(uint8_t rating);
};

#endif