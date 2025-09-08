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

#define MAX_TOKENS 3
#define MAX_COMMAND_LENGTH 128


void shell_init() {
    //start shell

    printf("\n");
    printf("Welcome to FS Shell!\n");

    while (1) {
        /*
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
        
        */
        printf("\n");
        printf("type 'help' for a list of commands\n");
        printf("----------------------\n");
        printf("SHELL> ");

        char command[MAX_COMMAND_LENGTH];

        fgets(command, MAX_COMMAND_LENGTH, stdin);

        //remove newline character
        command[strcspn(command, "\n")] = 0;

        if (strlen(command) == 0) {
            printf("Invalid input: no command entered\n");
            continue;
        }

        //tokenize command

        char* tokens[MAX_TOKENS];

        char* token = strtok(command, " \t");
        int i = 0;

        while (token != NULL && i < MAX_TOKENS) {
            tokens[i++] = token;
            // debug: printf("Token %d: %s\n", i, tokens[i - 1]);
            token = strtok(NULL, " \t");
        }

        //stop if too many tokens are present

        if (token != NULL) {
            printf("Error: too many arguments. Maximum is %d\n", MAX_TOKENS);
            continue;
        }

        tokens[i] = NULL;  // terminator
        char* comm = tokens[0];
        //switch case for first command

        if(strncmp(comm, "help", 4) == 0) {
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
            continue;
        }
        else if(strncmp(comm, "close", 5) == 0) {
            printf("Exiting shell...\n");
            break;
        }
        else if(strncmp(comm, "format", 6) == 0) {
            //format
            //check if name and size are provided
            if (tokens[1] == NULL || tokens[2] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* fs_filename = tokens[1];
            int size = atoi(tokens[2]);
            printf("Formatting disk...\n");
            printf("Filename: %s, Size: %d\n", fs_filename, size);
            continue;
        }
        else if(strncmp(comm, "mkdir", 6) == 0) {
            //mkdir
            //check if name is provided
            if (tokens[1] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* dir_name = tokens[1];
            printf("Creating directory...\n");
            printf("Created directory: %s\n", dir_name);
            continue;
        }
        else if(strncmp(comm, "cd", 2) == 0) {
            //cd
            printf("Changing directory...\n");
            continue;
        }
        else if(strncmp(comm, "touch", 5) == 0) {
            //touch
            //check if name is provided
            if (tokens[1] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* file_name = tokens[1];
            printf("Creating empty file...\n");
            printf("Created empty file: %s\n", file_name);
            continue;
        }
        else if(strncmp(comm, "append", 6) == 0) {
            //append
            //check if name and text are provided
            if (tokens[1] == NULL || tokens[2] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* file_name = tokens[1];
            char* text = tokens[2];
            printf("Appending to file: %s\n", file_name);
            printf("Text to append: %s\n", text);
            continue;
        }
        else if(strncmp(comm, "rm", 2) == 0) {
            //rm
            //check if name is provided
            if (tokens[1] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* file_name = tokens[1];
            printf("Removing file: %s\n", file_name);
            continue;
        }
        else if(strncmp(comm, "ls", 2) == 0) {
            //ls
            printf("Listing files...\n");
            continue;
        }
            else if(strncmp(comm, "cat", 3) == 0) {
                //cat
                //check if name is provided
                if (tokens[1] == NULL) {
                    printf("Error: missing arguments\n");
                    continue;
                }
                char* file_name = tokens[1];
                printf("Displaying content of file: %s\n", file_name);
                continue;
            }
        else {
            printf("Unknown command: %s\n", tokens[0]);
            continue;
        }

        
    }
    printf("shell closed\n");
}
