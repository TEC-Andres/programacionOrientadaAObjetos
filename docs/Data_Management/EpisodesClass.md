# Episodes Class

```mermaid
classDiagram
class Episodes{
    -uint32_t episodeNumber
    +Episodes()
    +Episodes(uint32_t id, string name, uint32_t length, string genre, uint8_t rating, uint32_t season, string title, uint32_t episodeNumber)
    +getEpisodeNumber() uint32_t
    +setEpisodeNumber(uint32_t) void
}
```

## module description
this document clarifies the low-level tracking schemas applied to map specific sub-elements tied to an inherited parent series container.

## private class members
* season - type uint32_t counter marking the specific season block placement.
* episodenumber - type uint32_t counter tracking the serial entry index of the chapter.
* title – type string title of the series

## object behavior
* inherits all properties from series and video.
* stores the specific episode number.
* allows episodes to be managed independently while remaining connected to their parent series.

## inheritance relationship
* episode is a specialized type of series.
* indirectly inherits all video attributes.
