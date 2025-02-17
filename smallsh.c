#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// Define Constants
#define INPUT_LENGTH 2048
#define MAX_ARGS 512
const char* comment = "#";

void command(void) {
	/*
	** Runs a command from the shell
	**
	** Code adapted from Canvas Exploration: Process API - Creating and Terminating Processes
	** Date: 02/16/2025
	** URL: https://canvas.oregonstate.edu/courses/1987883/pages/exploration-process-api-creating-and-terminating-processes?module_item_id=24956218
	*/
	pid_t spawnpid = -5;
	spawnpid = fork();

	switch(spawnpid) {
		case -1:
			// Error 
			perror("fork() failed");
			exit(EXIT_FAILURE);
			break;
		case 0:
			// Child process
			break;
		default:
			// Parent process

			// Wait for spawnid to terminate
			break;
	}


}

struct command_line
{
	char *argv[MAX_ARGS + 1];
	int argc;
	char *input_file;
	char *output_file;
	bool is_bg;
	bool is_empty;
	bool exit;
};

struct command_line *parse_input()
{
	/*
	** Code adapted from canvas
	** Date:
	** URL: 
	*/
	char input[INPUT_LENGTH];
	struct command_line *curr_command = (struct command_line *) calloc(1, sizeof(struct command_line));

	// Get input
	printf(": ");
	fflush(stdout);
	fgets(input, INPUT_LENGTH, stdin);

	// Tokenize the input
	char *token = strtok(input, " \n");

	// Check input for comment or blank line
	if (input[0] == '\n') {
		curr_command->is_empty = true;
		return curr_command;
	}
	else if (!strcmp(token, comment)) {
		curr_command->is_empty = true;
		return curr_command;
	}
	else {
		curr_command->is_empty=false;
	}

	while(token){
		if(!strcmp(token,"<")){
			curr_command->input_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			curr_command->output_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")){
			curr_command->is_bg = true;
		} else{
			curr_command->argv[curr_command->argc++] = strdup(token);
		}
		token=strtok(NULL," \n");
	}
	

	// Check for Shell Built-in Commands
	if (!strcmp(curr_command->argv[0], "exit")) {
		curr_command->exit = true;
		// Kill all processes/jobs that have been created by the shell
	}
	else if (!strcmp(curr_command->argv[0], "cd") {
		// Implement cd func
	}

	else if (!strcmp(curr_command->argv[0], "status") {
		// Implement status func
	}
	else {
		// Run built in command 
		command();

	}

	return curr_command;
}

int main()
{
	struct command_line *curr_command;

	while(true)
	{
		curr_command = parse_input();
		if (curr_command->is_empty) {
			free(curr_command);
			continue;
		}
		else if (curr_command->exit) {
			return EXIT_SUCCESS;
		}
		else {
			printf("Finsihed");
		}
	}
	return EXIT_SUCCESS;
}