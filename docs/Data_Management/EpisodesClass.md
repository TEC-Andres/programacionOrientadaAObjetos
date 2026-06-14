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

## structural behavior
* individual episodes embed directly into tracking arrays inside their series parents while maintaining access to foundational parent video fields via inheritance chains.
