# Series Class

```mermaid
classDiagram
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
```

## module description
this document outlines the secondary derived structural layout used to bundle tv show titles before dividing contents into episode structures.

## private class members
* title - type string holding the global parent heading for the full show container.

## public operations
* connects parameter variables back to base video constructor parameters.
* provides public getTitle() and setTitle() methods to manage show data safely.
