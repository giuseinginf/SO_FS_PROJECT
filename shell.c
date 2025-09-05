/*
The shell will show a command prompt and allow the user to enter commands. The commands are:
- format <fs_filename> <size>
   - mkdir
   - cd
   - touch (creates an empty file)
   - cat (prints the content of a file to the screen)
   - ls
   - append <file> <text>
   - rm <dir/file>
   - close

   The shell consists in a while loop that continuously prompts the user for a command, 
   reads the command, and executes it. The loop will terminate when the user enters the "exit" command.

*/

#include "shell.h"

int is_command_available(const char *command, char *available_commands[]) {
    for (int i = 0; available_commands[i] != NULL; i++) {
        if (strcmp(command, available_commands[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void shell_init() {
    //start shell
    while (1) {
        char* command = malloc(MAX_COMMAND_LENGTH);
        if (!command) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }

        //show commands
        printf("\n");
        printf("----------------------\n");
        printf("\n");
        printf("Available commands:\n");
        printf(" - format <fs_filename> <size>\n");
        printf(" - mkdir <dir_name>\n");
        printf(" - cd <dir_name>\n");
        printf(" - touch <file_name>\n");
        printf(" - cat <file_name>\n");
        printf(" - ls\n");
        printf(" - append <file> <text>\n");
        printf(" - rm <dir/file>\n");
        printf(" - close\n");
        printf("\n");
        printf("----------------------\n");
        printf("\n");

        printf("SHELL> ");

        fgets(command, MAX_COMMAND_LENGTH, stdin);
        
        if (strcmp(command, "exit") == 0) {
            break;
        }

        char *available_commands[] = {
            "format",
            "mkdir",
            "cd",
            "touch",
            "cat",
            "ls",
            "append",
            "rm",
            "close"
        };

        if (is_command_available(command, available_commands)) {

            if (strcmp(command, "format") == 0) {
                //format
                printf("Formatting disk...\n");
                continue;
            } else if (strcmp(command, "mkdir") == 0) {
                //mkdir
                printf("Creating directory...\n");
                continue;
            } else if (strcmp(command, "cd") == 0) {
                //cd
                printf("Changing directory...\n");
                continue;
            } else if (strcmp(command, "touch") == 0) {
                //touch
                printf("Creating empty file...\n");
                continue;
            } else if (strcmp(command, "append") == 0) {
                //append
                printf("Appending to file...\n");
                continue;
            } else if (strcmp(command, "rm") == 0) {
                //rm
                printf("Removing file...\n");
                continue;
            } else if (strcmp(command, "ls") == 0) {
                //ls
                printf("Listing files...\n");
                continue;
            }

        }
    }
    printf("shell closed\n");
}
