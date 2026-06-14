# Mapping UI Elements

## module description
this section documents how individual components are mapped to the screen, focusing on responsive partitioning and input routing.

## core components
* `ui::MapComponent` Class - the central event loop and layout manager. maintains a grid of components via the internal `GridCell` structure and a list of render callbacks.
* focus & navigation - handles raw keyboard inputs and translates them into spatial navigation. uses distance calculations and alignment bands to intelligently pass the "selected" state to the nearest focusable component.
* event loop - the `run()` method initializes the application, sets up alternate screen buffers, establishes non-blocking input states, and loops continuously. selectively redraws dirty components to optimize performance.
* `ui::Partition` Class - allows the terminal space to be mathematically divided into distinct regions. supports standard division profiles defined by the `Type` enum.
* `Partition::Region` - each partition yields `Region` objects where `IComponent` elements can be attached. regions process their own background colors and utilize alignment bands to vertically stack components while preventing overlaps.