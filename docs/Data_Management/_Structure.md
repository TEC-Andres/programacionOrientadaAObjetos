# Data Structure

This document outlines the data structure used in the application, including the format and organization of data for different components.

## Overview

```mermaid
classDiagram
class Video{
    -uint32_t id
    -string name
    -uint32_t length
    -string genre
    -uint8_t rating
    +Video()
    +Video(uint32_t id, string name, uint32_t length, string genre, uint8_t rating)
    +getId() uint32_t
    +setId(uint32_t id) void
    +getName() string
    +setName(string name) void
    +getLength() uint32_t
    +setLength(uint32_t length) void
    +getGenre() string
    +setGenre(string genre) void
    +getRating() uint8_t
    +setRating(uint8_t rating) void
}

class Movie{
    -string director
    +Movie()
    +Movie(uint32_t id, string name, uint32_t length, string genre, uint8_t rating, string director)
    +getDirector() string
    +setDirector(string director) void
}

class Series{
    -uint32_t season
    -string title
    +Series()
    +Series(uint32_t id, string name, uint32_t length, string genre, uint8_t rating, uint32_t season, string title)
    +getSeason() uint32_t
    +setSeason(uint32_t season) void
    +getTitle() string
    +setTitle(string title) void
}

class Episodes{
    -uint32_t episodeNumber
    +Episodes()
    +Episodes(uint32_t id, string name, uint32_t length, string genre, uint8_t rating, uint32_t season, string title, uint32_t episodeNumber)
    +getEpisodeNumber() uint32_t
    +setEpisodeNumber(uint32_t) void
}

Video <|-- Movie : Inherits
Video <|-- Series : Inherits
Series <|-- Episodes : Inherits
```

This file contains the complete class diagram for the system. It shows all four classes (Video, Movie, Series, Episode) and the inheritance relationships between them.

Inheritance establishes an "is-a" relationship:

- A Series is a type of Video.
- A Movie is a type of Video.
- An Episode is a type of Series (and therefore also a type of Video)

Everything that is true for a Video (having an ID, name, length, genre, and rating) is also true for Series, Movie and Episode.

Three-level hierarchy:
- Level 1: Video - the most general class
- Level 2: Series and Movie - concrete types of video
- Level 3: Episode - an even more specific type of series

Thanks to inheritance, child classes do not need to redefine the attributes and methods that already exist in the parent class. This avoids code duplication and makes maintenance easier.

## module description
this module manages the data storage blueprints, object hierarchy logic, and custom container models used to map multimedia structures in program memory.

the inheritance structure is organized as follows:
* video is the base class.
* movie inherits from video.
* series inherits from video.
* episodes inherits from series.

## relationship overview
* a movie is a type of video.
* a series is a type of video.
* an episode is a type of series and also a type of video.

## benefits of the structure
* avoids duplicating common attributes across classes.
* simplifies maintenance and future updates.
* keeps related information organized through inheritance.
* improves code readability and scalability.

## included files
* video class (VideoClass.md) - data fields for the generic base class entity.
* movie class (MovieClass.md) - layout specifications for film items.
* series class (SeriesClass.md) - architectural design for show models.
* episodes class (EpisodesClass.md) - individual tracking items for episodes.
* dynamic arrays (DynamicArrayHandling.md) - runtime heap memory array controls.
