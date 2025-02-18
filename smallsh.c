#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

// Define Constants
#define INPUT_LENGTH 2048
#define MAX_ARGS 512
const char* comment = "#";

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

void command(struct command_line* curr_command) {
	/*
	** Runs a command from the shell
	**
	** Code adapted from Canvas Exploration: Process API - Creating and Terminating Processes
	** Date: 02/16/2025
	** URL: https://canvas.oregonstate.edu/courses/1987883/pages/exploration-process-api-creating-and-terminating-processes?module_item_id=24956218
	*/

	
	pid_t spawnpid = -5;
	pid_t childpid;
	spawnpid = fork();
	int childStatus;
	int execProgram;


	switch(spawnpid) {
		case -1:
			// Error 
			perror("fork() failed");
			exit(EXIT_FAILURE);
			break;
		case 0:
			// Child process
			execvp(curr_command->argv[0], curr_command->argv);
			printf("%s: no such file or directory\n", curr_command->argv[0]);
			exit(EXIT_FAILURE);

		default:
			// Parent process

			// Wait for spawnid to terminate
			childpid = waitpid(spawnpid, &childStatus, 0);
			if (childpid == -1) {
				perror("waitpid() failed");
			}
			break;
	}
}

void cd(struct command_line* curr_command) {
	/*
	** Changes working directory of smallsh
	** 
	*/
	int chdirResult; // Stores result of directory change 

	// No arguments, change directory to directory specified in HOME environment variable
	if (curr_command->argv[1] == NULL) {
		// Get home environment variable 
		char* home;
		home = getenv("HOME");
		if (home == NULL) {
			printf("HOME env variable not found");
		}
		else {
			chdirResult = chdir(home);
			if (chdirResult != 0) {
				printf("Error changing directory");
			}
		}
	}
	// One argument - Path of directory to change to, either relative or absolute
	else {
		chdirResult = chdir(curr_command->argv[1]);
		if (chdirResult != 0) {
			printf("Error changing directory");
		}
	}
}

struct command_line *parse_input()
{
	/*
	** Code adapted from canvas
	** Date:
	** URL: 
	*/
	char input[INPUT_LENGTH] = {0};
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
	else if (!strcmp(token, comment) || token[0] == '#') {
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
	curr_command->argv[curr_command->argc] = NULL;
	
	// Check for Shell Built-in Commands
	if (!strcmp(curr_command->argv[0], "exit")) {
		curr_command->exit = true;
		// Kill all processes/jobs that have been created by the shell
	}
	else if (!strcmp(curr_command->argv[0], "cd")) {
		// Implement cd func
		cd(curr_command);
	}

	else if (!strcmp(curr_command->argv[0], "status")) {
		// Implement status func
	}
	else {
		// Run built in command 
		command(curr_command);

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
			free(curr_command);
			continue;
		}
	}
	return EXIT_SUCCESS;
}