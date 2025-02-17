#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

	if (input[0] == '\n') {
		curr_command->is_empty = true;
		return curr_command;
	}

	if (!strcmp(token, comment)) {
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
		else {
			int count = 0;
			char* curr_input = curr_command->argv[count];
			while(curr_input != NULL)  {
				printf("Curr input: %s\n", curr_input);
				count++;
				curr_input = curr_command->argv[count];
			}	
		}
	}
	return EXIT_SUCCESS;
}