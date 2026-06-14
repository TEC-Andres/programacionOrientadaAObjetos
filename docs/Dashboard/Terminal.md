# Terminal in Dashboard

## Terminal usage
The terminal provides a command-line interface for users to interact with the application. It allows users to execute commands, view logs, and perform various administrative tasks.

All commands need to be prefixed with a colon (:) to be recognized by the terminal.

Here are all of the functions available in the terminal:
```
:table <movies|series|episodes>
:tableCLI <movies|series|episodes>
:show <type> <id>
:graph histogram <rating|length> <N|\"title1\" \"title2\">
:graph pieChart <rating|length> <N|\"title1\" \"title2\">
:graph gradsquares <seriesName>
:cleargraph
:get <name|rating|length|size> [type] [id]
:set <name|rating|length> <type> <id> <value>
:add <movie|series|episode> <id> <args...>
:remove <movie|series|episode> <id>
:showobjectsinuse
:save
:clear
```

These commands allow the user to do direct changes to the database, create graphs of the data, and manage the application's state.