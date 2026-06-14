# UI Controllers

# Background Component

## module description
this module provides a non-focusable component that fills the entire terminal with a solid background color using OSC 11 escape sequences.

## data structures
* background color - uses a string `color_` to store the hex color value for the background.

## core routines
* `toString` - generates a string consisting of multiple lines of spaces with the background color applied, determining the number of lines and width based on the current console dimensions.
* `setColor` - updates the hex color string utilized by the background component.

***

# Button Component

## module description
this module provides a focusable component that displays a clickable button which can be selected and activated by the user.

## data structures
* button state - stores properties including `text_`, styling hex colors for the background, foreground, and border, a boolean `selected_` state, and an `onActivate_` callback function.

## core routines
* `toString` - visually represents the button with its text, colors, padding, and styling, altering its background to `#3366cc` and foreground to `#ffffff` when the button is in a selected state.
* `setOnActivate` - assigns a callback function to be executed when the button is activated.
* `setSelected` - updates the focus state of the button and flags the component as dirty if the state is altered.

***

# LocationBar Component

## module description
this module provides a non-focusable location bar component that displays up to three distinct text segments, each supporting separate alignments.

## data structures
* segment - uses a `Segment` structure containing the visibility state (SHOW or HIDE), text content, and alignment preference (left, center, or right) for each part of the location bar.

## core routines
* `placeText` - a static helper function that inserts segment text into the location bar's string buffer at a calculated position while preventing buffer overflow.
* `toString` - calculates alignment positions and generates a string buffer visually representing the location bar's visible segments.
* `render` - generates the string representation of the location bar and outputs it directly to a provided output stream while correctly matching the console width.

***