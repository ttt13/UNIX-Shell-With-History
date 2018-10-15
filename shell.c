// Shell starter file
// You may make any changes to any part of this file.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_LENGTH 10

char history[HISTORY_LENGTH][COMMAND_LENGTH];
int numberInHistory = 0;
int totalInHistory = 0;

void historyTracker(){ // Display only previous 10 commands
	_Bool hasHistory = true;
	int i = 0;
	int low = 1;
	int remainingHistory = totalInHistory; 
	if (totalInHistory == 0){
		return;
	}

	if(totalInHistory > 10){
		remainingHistory = 10;
		low = totalInHistory - 9;
	}

	while (i < HISTORY_LENGTH && hasHistory){
		printf("%d\t", low);
		printf("%s", history[i]);

		i++;
		low++;
		remainingHistory--;
		printf("\n");

		if (remainingHistory <= 0){
			hasHistory = false;
		}
	}

}
/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */

void addToHistory(char *buff){
	char temporaryHistory[HISTORY_LENGTH][COMMAND_LENGTH];

	// For this, history[9] is the most recent item.
	// This is for later convenience when printing out the history.
	if (totalInHistory > 10){
		for(int i = 0; i < 9; i++){
			strcpy(temporaryHistory[i], history[i+1]);
		}
		for(int j = 0; j < 9; j++){
			strcpy(history[j], temporaryHistory[j]);
		}
		strcpy(history[9], buff);
	}
	else if (totalInHistory <= 10){
		strcpy(history[totalInHistory-1], buff);
	}
}

int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;

	_Bool shouldIAddToHistory = true;


	if( buff[0] != '\0' ){
		if(buff[0] == '!' && buff[1] == '!' && buff[2] == '\0'){
		// Two cases: if totalInHistory > 10, or not.
			if(totalInHistory == 0){
				write(STDOUT_FILENO, "SHELL: Unknown history command.\n", strlen("SHELL: Unknown history command.\n"));
				shouldIAddToHistory = false;
			}
			if(totalInHistory > 10){

				strcpy(buff, history[9]);
				write(STDOUT_FILENO, buff, strlen(buff));
				write(STDOUT_FILENO, "\n", strlen("\n"));
			}
			else if(totalInHistory <= 10 && totalInHistory > 0){
				strcpy(buff, history[totalInHistory-1]);
				write(STDOUT_FILENO, buff, strlen(buff));
				write(STDOUT_FILENO, "\n", strlen("\n"));
			}
		}

		else if(buff[0] == '!'){
			char changeToInt[20];
			int traverseIndex = 1;

			while(buff[traverseIndex] != '\0'){
				changeToInt[traverseIndex - 1] = buff[traverseIndex];
				traverseIndex++;
			}

			int getInt = atoi(changeToInt);
			int topIndex;
			int lowIndex; 

			if(getInt == 0){
				write(STDOUT_FILENO, "SHELL: Unknown history command.\n", strlen("SHELL: Unknown history command.\n"));
				shouldIAddToHistory = false;
			}

			if(totalInHistory <= 10 && getInt != 0){
				if(getInt <= totalInHistory){
					strcpy(buff, history[getInt-1]);
					write(STDOUT_FILENO, buff, strlen(buff));
					write(STDOUT_FILENO, "\n", strlen("\n"));
				}
				else if (getInt > totalInHistory){
					write(STDOUT_FILENO, "SHELL: Unknown history command.\n", strlen("SHELL: Unknown history command.\n"));
					shouldIAddToHistory = false;
				}
			}
			else if (totalInHistory > 10){
				topIndex = totalInHistory;
				lowIndex = totalInHistory - 9;
				int findIndex = 0;
				_Bool outOfRange = false;

				if(getInt > topIndex || getInt < lowIndex){
					write(STDOUT_FILENO, "SHELL: Unknown history command.\n", strlen("SHELL: Unknown history command.\n"));
					shouldIAddToHistory = false;
					outOfRange = true;
				}

				while(lowIndex <= topIndex && lowIndex != getInt){
					lowIndex++;
					findIndex++;
				}

				if (outOfRange == false){
					strcpy(buff, history[findIndex]);
					write(STDOUT_FILENO, buff, strlen(buff));
					write(STDOUT_FILENO, "\n", strlen("\n"));
				}
			}
		}
	}
	else if(buff[0] == '\0'){
		shouldIAddToHistory = false;
	}
	
	if(shouldIAddToHistory){
		totalInHistory++;
		addToHistory(buff);
	}
	
	int num_chars = strnlen(buff, COMMAND_LENGTH);
	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */


void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

	if(length < 0 && errno == EINTR){
		buff[0] = '\0';
	}

	if ( (length < 0) && (errno !=EINTR) ){
    	perror("Unable to read command. Terminating.\n");
    	exit(-1);  /* terminate with error */
	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}

void handle_SIGINT(){
	write(STDOUT_FILENO, "\n", strlen("\n"));
	historyTracker();
}


/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{
	pid_t pid;
	char cwd[COMMAND_LENGTH];
	char internalpwd[COMMAND_LENGTH];
	char input_buffer[COMMAND_LENGTH]; // This holds the input
	char *tokens[NUM_TOKENS]; // The max # arguments
	int cdExistence;

	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	handler.sa_flags = 0;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);

	while (true) {

		_Bool internalCommand = false;

		getcwd(cwd, sizeof(cwd));
		write(STDOUT_FILENO, cwd, strlen(cwd));
		write(STDOUT_FILENO, "$ ", strlen("$ "));
		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().
		
		_Bool in_background = false;

		read_command(input_buffer, tokens, &in_background);
		
		numberInHistory++; // Keep track of number.

		if(tokens[0]){

			if(strcmp(tokens[0], "exit") == 0){		 
				exit(0);
			}

			else if(strcmp(tokens[0], "pwd") == 0){

				getcwd(internalpwd, sizeof(internalpwd));
				write(STDOUT_FILENO, internalpwd, strlen(internalpwd));
				write(STDOUT_FILENO, "\n", strlen("\n"));
				internalCommand = true;
			}

			else if(strcmp(tokens[0], "cd") == 0){
				cdExistence = chdir(tokens[1]);

				if(cdExistence == -1){
					write(STDOUT_FILENO, "Invalid directory.\n", strlen("Invalid directory.\n"));
				}
				internalCommand = true;
			}

			else if(strncmp(tokens[0], "!", 1) == 0){
				internalCommand = true;
			}

			else if(strcmp(tokens[0], "type") == 0){
				if(tokens[1] != NULL){
					int a = strcmp(tokens[1], "exit");
					int b = strcmp(tokens[1], "pwd");
					int c = strcmp(tokens[1], "cd");
					int d = strcmp(tokens[1], "type");

					if(a == 0 || b == 0 || c == 0 || d == 0){
						write(STDOUT_FILENO, tokens[1], strlen(tokens[1]));
						write(STDOUT_FILENO, " is a shell300 builtin\n", strlen(" is a shell300 builtin\n"));			
					}

					else{
						write(STDOUT_FILENO, tokens[1], strlen(tokens[1]));
						write(STDOUT_FILENO, " is external to shell300\n", strlen(" is external to shell300\n"));
					}
				}
				else{
					write(STDOUT_FILENO, "Type not specified.\n", strlen("Type not specified.\n"));
				}
				internalCommand = true;
			}

			else if(strcmp(tokens[0], "history") == 0){
				historyTracker();
				internalCommand = true;
			}
		}

		if(internalCommand == false){
			pid = fork();

			if(pid < 0){
				perror("Failure to fork.\n");
				exit(-1);
			}
			else if (pid == 0){
				int checkValidity = execvp(tokens[0], tokens);
				if (checkValidity == -1){
					write(STDOUT_FILENO, tokens[0], strlen(tokens[0]));
					write(STDOUT_FILENO, ": Unknown command.\n", strlen(": Unknown command.\n"));
					exit(0);
				}
			}
			else if (pid > 0){

				if(in_background == false){
					waitpid(pid, NULL, 0);
				}
			}
		}
/*		// DEBUG: Dump out arguments:
		for (int i = 0; tokens[i] != NULL; i++) {
			write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
			write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
		if (in_background) {
			write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));
		}
*/
		/**
		 * Steps For Basic Shell:
		 * 1. Fork a child process
		 * 2. Child process invokes execvp() using results in token array.
		 * 3. If in_background is false, parent waits for
		 *    child to finish. Otherwise, parent loops back to
		 *    read_command() again immediately.
		 */

	}
	return 0;
}