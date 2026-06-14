# UI Modals
# DialogBox Component

## module description
this module manages a focusable dialog box component that displays a title, a message body, and interactive buttons to prompt the user for input or display messages.

## data structures
* dialog button - utilizes a `DialogButton` structure to track the string text and boolean selection state of individual buttons.

## core routines
* `addButton` - appends a new button with specified text and selection state to the dialog box's internal list.
* `clearButtons` - removes all existing buttons from the dialog box.
* `toString` - generates a string visually representing the dialog box by rendering the title, wrapping the message text, and dynamically spacing the buttons across the bottom row.

***

# MessageBox Component

## module description
this module implements a non-focusable message box component designed to show information or alerts to the user through a styled title and message body.

## data structures
* message state - stores strings for the `title_` and `message_`, alongside specific hex strings for the title background, title foreground, body background, body foreground, and border colors.

## core routines
* `toString` - builds the visual representation of the message box by drawing borders, centering the title text, padding with spaces, and wrapping the message body lines to fit within the specified content width.
* `setTitle` - updates the text displayed in the title area of the message box.
* `setMessage` - updates the text displayed in the main body of the message box.