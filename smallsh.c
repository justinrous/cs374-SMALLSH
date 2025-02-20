#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


// Define Constants
#define INPUT_LENGTH 2048
#define MAX_ARGS 512
const char* comment = "#";

int exitStatus = 0;

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

int command(struct command_line* curr_command) {
	/*
	** Runs a command from the shell
	**
	** Code adapted from Canvas Exploration: Process API - Creating and Terminating Processes
	** Date: 02/16/2025
	** URL: https://canvas.oregonstate.edu/courses/1987883/pages/exploration-process-api-creating-and-terminating-processes?module_item_id=24956218
	*/

	pid_t spawnpid;
	pid_t childpid;
	spawnpid = fork();
	int childStatus;
	int execProgram;

	// If input File, get data from input file 


	switch(spawnpid) {
		case -1:
			// Error 
			perror("fork() failed");
			exit(EXIT_FAILURE);
			break;
		case 0:
			// Child process

			// If input_file, redirect standard input 
			if (curr_command->input_file) {
				int inputFD = open(curr_command->input_file, O_RDONLY, 0644);

				// if can't open file, set exit status to 1
				if (inputFD == -1) {
					printf("cannot open %s for input\n", curr_command->input_file);
					exit(EXIT_FAILURE);
				}
				else {
					int inputRedirect = dup2(inputFD, STDIN_FILENO);
					if (inputRedirect == -1) {
						printf("Error setting standard input");
						exit(EXIT_FAILURE);
					}
				}
			}

			// If output_file, redirect standard output 
			if (curr_command->output_file) {
				int outputFD = open(curr_command->output_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);

				//
				if (outputFD == -1) {
					exit(EXIT_FAILURE);
				}
				else {
					int outputRedirect = dup2(outputFD, STDOUT_FILENO);
					if (outputRedirect == -1) {
						printf("Error setting stdout");
						exit(EXIT_FAILURE);
					}
				}
			}

			execvp(curr_command->argv[0], curr_command->argv);
			exit(EXIT_FAILURE);

		default:
			// Parent process

			// Wait for spawnid to terminate
			waitpid(spawnpid, &childStatus, 0);
			if (WIFEXITED(childStatus)) {
				int exitCode = WEXITSTATUS(childStatus);
				if (exitCode == 1) {
					exitStatus = 1;
				}
				else {
					exitStatus = 0;
				}
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
			printf("HOME env variable not found\n");
		}
		else {
			chdirResult = chdir(home);
			if (chdirResult != 0) {
				printf("no such file or directory\n");
			}
		}
	}
	// One argument - Path of directory to change to, either relative or absolute
	else {
		chdirResult = chdir(curr_command->argv[1]);
		if (chdirResult != 0) {
			printf("Error changing directory\n");
		}
	}
}

void status(void) {
	printf("exit value %d\n", exitStatus);
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

	// Check input for comment or blank line
	if (input[0] == '\n' || input[0] == '#') {
		curr_command->is_empty = true;
		return curr_command;
	}
	else {
		curr_command->is_empty=false;
	}

	// Tokenize the input
	char *token = strtok(input, " \n");

	while(token){
		if(!strcmp(token,"<")){
			curr_command->input_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			curr_command->output_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")){
			curr_command->is_bg = true;
		} 
		else{
			curr_command->argv[curr_command->argc++] = strdup(token);
		}
		token=strtok(NULL," \n");
	}
	return curr_command;
}

void processCommands(struct command_line* curr_command) {
	// Check for Shell Built-in Commands
	if (!strcmp(curr_command->argv[0], "exit")) {
		curr_command->exit = true;
		return;
		// Kill all processes/jobs that have been created by the shell
	}
	else if (!strcmp(curr_command->argv[0], "cd")) {
		// Implement cd func
		cd(curr_command);
	}
	else if (!strcmp(curr_command->argv[0], "status")) {
		// Implement status func
		status();
	}
	else {
		// Run built in command 
		command(curr_command);
	}
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

		processCommands(curr_command);

		if (curr_command->exit) {
			// Kill all processes/jobs that have been created by the shell
			exit(EXIT_SUCCESS);
		}
		else {
			free(curr_command);
			continue;
		}
	}
	return EXIT_SUCCESS;
}