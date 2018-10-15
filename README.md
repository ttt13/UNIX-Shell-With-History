# UNIX-Shell-With-History

This shell accepts user commands and then executes each command in a separate process. It provides the user a prompt at which the next command is entered.

# Internal commands

Several commands are commands that are implemented by the shell itself, and is handled directly in the parent process without a fork:
exit: Exits the program
cd: Changes directory if valid
pwd: Displays the current working directory.
type: Displays information about the command type.

# History

The history displays the recent 10 commands. The history command can be accessed in several ways:
Through the internal command 'history'
Using '!n': The command '!n' runs command number n from the history. 
Using '!!': Runs the most recent command from the history.

# Signals

Ctrl-c: This will display the history.

# Makefile

Compiles shell.c. Run as ./shell
