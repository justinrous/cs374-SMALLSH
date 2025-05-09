#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>


// Define Constants
#define INPUT_LENGTH 2048
#define MAX_ARGS 512
const char* comment = "#";

int exitStatus = 0;
int badInputFile = 0; // Stores 1 if invalid command is entered - 0 if valid
bool sig_tstp = false; // Stores true if SIGSTP signal is sent. False if background commands are allowed. 

pid_t bg_processes[1000] ={-1}; // Stores pointers for background process ids

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

void handle_SIGTSTP(int signo) {
	/*
	** Signal handler for parent process when SIGTSTP signal is sent 
	*/
	if (!sig_tstp) {
		char* sigstp_message = "\nEntering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, sigstp_message, 50);
		sig_tstp = true;
	}
	else {
		char* sigstp_message_1 = "\nExiting foreground-only mode\n";
		write(STDOUT_FILENO, sigstp_message_1, 29);
		sig_tstp = false;
	}
}


void check_bg_processes(void) {
	/*
	** This function is called by the parent function to check for any background child  
	** processes that have exited. Updates exitStatus for the background process 
	** and checks if background process was terminated by a signal. 
	*/

	int childStatus;
	pid_t childRes;
	pid_t childPid;

	for (int i = 0; i < 1000; i++) {
		if (bg_processes[i] > -1) {
			childPid = bg_processes[i];
			childRes = waitpid(childPid, &childStatus, WNOHANG);
			if (childRes > 0) {
				if (WIFEXITED(childStatus)) {
					int exitCode = WEXITSTATUS(childStatus);
					if (exitCode == 1) {
						exitStatus = 1;
					}
					else {
						exitStatus = 0;
					}
					printf("background pid %d is done; exit value %d\n", bg_processes[i], exitStatus);
				}
				else if (WIFSIGNALED(childStatus)) {
					int signo = WTERMSIG(childStatus);
					printf("background pid %d is done; terminated by signal %d\n", bg_processes[i], signo);
				}
				// Reset index of array that stored exited background process
				bg_processes[i] = -1;
			}
		}
	}
}

int command(struct command_line* curr_command) {
	/*
	** This function is used to run a non-bulit in command. The parent process forks off a child process and updates stdin/stdout
	** for the child process. 
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

	switch(spawnpid) {
		case -1:
			// Error 
			perror("fork() failed");
			exit(EXIT_FAILURE);
			break;
		case 0:
			// Child process
			if (curr_command->is_bg) {
				int bg_pid = (int) getpid();
				printf("background pid is %d\n", bg_pid);
			}
			else {
				// Restore default SIGINT action in child 
				struct sigaction fg_sigINT_action;
				fg_sigINT_action.sa_handler = SIG_DFL;
				sigaction(SIGINT, &fg_sigINT_action, NULL);
			}

			// Register SIGTSTP handler for child to ignore SIGTSTP
			struct sigaction SIGTSTP_action = {0};
			SIGTSTP_action.sa_handler = SIG_IGN;
			// SIGTSTP_action.sa_flags = SA_RESTART;
			sigaction(SIGTSTP, &SIGTSTP_action, NULL);

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
						printf("Error setting standard input\n");
						exit(EXIT_FAILURE);
					}
				}
			}
			// If background process with no input file, set standard input to "/dev/null"
			else if (curr_command->is_bg) {
				int inputFD = open("/dev/null", O_RDONLY, 0644);

				// if can't open file, set exit status to 1
				if (inputFD == -1) {
					printf("cannot open %s for input\n", curr_command->input_file);
					exit(EXIT_FAILURE);
				}
				else {
					int inputRedirect = dup2(inputFD, STDIN_FILENO);
					if (inputRedirect == -1) {
						printf("Error setting standard input\n");
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
			// If background process with no provided output file, set standard output to "/dev/null"
			else if (curr_command->is_bg) {
				int outputFD = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT, 0644);

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
			printf("%s: no such file or directory\n", curr_command->argv[0]);

			exit(EXIT_FAILURE);

		default:
			// Parent process

			// If not background process, wait for child to terminate and update exitStatus
			if (!curr_command->is_bg) {
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
				else if (WIFSIGNALED(childStatus)) {
					int signo = WTERMSIG(childStatus);
					printf("terminated by signal %d\n", signo);
				}
			}
			else {
				// Store process id (spawnid) in next available index of bg_processes array 
				for (int i = 0; i < 1000; i++) {
					// If null
					if (bg_processes[i] <= 0) {
						bg_processes[i] = spawnpid;
						break;
					}
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
	** Code adapted from code provided for assignment
	** Date: 02-23-25
	** URL: https://canvas.oregonstate.edu/courses/1987883/assignments/9864854?module_item_id=25049863
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

	if (!strcmp(token, "exit")) {
		curr_command->exit = true;
		return curr_command;
	}

	while(token){
		if(!strcmp(token,"<")){
			curr_command->input_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			curr_command->output_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&") && (!sig_tstp)){
			curr_command->is_bg = true;
		} else if (!strcmp(token, "&") && sig_tstp) {
			token = strtok(NULL, " \n");
			continue;
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
		// Register Signal Handler for Sigint
		/* Code adapted from Canvas Module
		** URL: https://canvas.oregonstate.edu/courses/1987883/pages/exploration-signal-handling-api?module_item_id=24956227
		** Date: 
		*/

		// Set parent process to ignore SIGINT
		struct sigaction SIGINT_action = {0};
		SIGINT_action.sa_handler = SIG_IGN;
		SIGINT_action.sa_flags = SA_RESTART;
		sigaction(SIGINT, &SIGINT_action, NULL);

		// Set sigaction for SIGTSTP in parent 
		struct sigaction SIGTSTP_action = {0};
		SIGTSTP_action.sa_handler = handle_SIGTSTP;
		SIGTSTP_action.sa_flags = SA_RESTART;
		sigaction(SIGTSTP, &SIGTSTP_action, NULL);

		check_bg_processes();
		curr_command = parse_input();

		if (curr_command->is_empty) {
			curr_command = NULL;
		}
		else if (curr_command->exit) {
			// Kill all processes/jobs that have been created by the shell
			for (int i = 0; i < 1000; i++) {
				if (bg_processes[i] > 0) {
					kill(bg_processes[i], SIGTERM);
				}
			}
			free(curr_command);
			exit(EXIT_SUCCESS);
		}
		else {
			processCommands(curr_command);
		}
		curr_command = NULL;
	}
	free(curr_command);
	return EXIT_SUCCESS;
}