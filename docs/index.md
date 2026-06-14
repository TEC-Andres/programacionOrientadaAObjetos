# Welcome to the Problem Situation Documentation!
This documentation provides an overview of the problem situation, including the main components and their interactions. It is intended to help developers understand the structure and functionality of the application.

### general description
this website contains all the technical documentation for the interactive video management system developed for the object oriented programming course at Tec de Monterrey. the main goal of this application is to display and manage information regarding movies, series, and episodes using a command line interface (cli).

the project is built with a modular approach to allow for easy maintenance and scalability, applying concepts such as inheritance and polymorphism.

### core architecture
the backend design is split into specific modules for data handling, security, and interface design.

below is the class diagram representing the inheritance model for the video objects:

```
classDiagram
    class Video{
        -uint32_t id
        -string name
        -uint32_t length
        -string genre
        -uint8_t rating
    }
    class Movie{
        -string director
    }
    class Series{
        -string title
    }
    class Episode{
        -uint32_t season
        -uint32_t episodeNumber
    }
    Video <|-- Movie : inherits
    Video <|-- Series : inherits
    Series <|-- Episode : inherits
```
## documentaion map

### core data & logic
handles the data models, base class layouts, and memory allocation frameworks.
video class models - specifications of the base video properties.

movies implementation - movie specific class fields and getters.

series & episodes structure - hierarchical coupling between series and episode classes.

dynamic arrays - execution logic for tracking objects in runtime memory.

data management index - overview of the data architecture.

### user access and security

database decryption - parsing routines for local data files.

environment settings - working variables and configuration files.

sha256 security - password hashing implementations.

elliptic curve crypto - advanced cryptographic logic.

login framework index - layout of the security subsystem.

### presentation (ux-ui)

terminal window controls - base layouts and terminal handling.

data tables presentation - matrix display of records with ansi coloring.

charts & telemetry - rendering telemetry inside the cli.

ui controllers - runtime control loops for user interface screens.

modals & updates - dynamic modal windows and screen refreshing logic.

dashboard index & ux structure - navigation entrypoints.

### project operations and workflows

compilation guide - building binaries locally using cmake 3.10+.

execution paths - deployment instructions for compatible platforms.

git protocols - branch naming conventions and lifecycle rules.

github rules - managing issues and submitting pull requests.
