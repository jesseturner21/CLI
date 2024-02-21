#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h> // For isspace()

static void run_prompt_loop(void);
static int run_batch_script(FILE* restrict batch_script);
static ssize_t parse_commands(char* restrict str, char*** restrict commands);
static int run_commands(char** commands, ssize_t command_count);

int main(int argc, char** argv) {
    switch (argc) {
    case 1:
        // Run the interactive command prompt tloop
        run_prompt_loop();
        break;
    case 2: {
        // Open the batch script
        FILE* batch_script = fopen(argv[1], "r");
        if (batch_script == NULL) {
            perror("open batch file");
            return 1;
        }

        // Execute the batch script
        int ret = run_batch_script(batch_script);
        if (ret == -1) {
            perror("run batch script");
            return -1;
        }

        // Close the batch script once we're done using it
        if (fclose(batch_script) == EOF) {
            perror("close batch file");
            return 1;
        }

        break;
    }
    default:
        // The user entered an incorrect number of arguments, so print usage information
        fprintf(stderr, "error: usage is '%s [batch script]'\n", argv[0]);
        return 1;
    }
}

static void run_prompt_loop(void) {
    while (true) {
        // Print the prompt
        printf("> ");

        // Read a line from the user
        char* line = NULL;
        size_t line_len = 0;
        if (getline(&line, &line_len, stdin) == -1) {
            // Error encountered reading the line (possibly EOF encountered), so quit the program.
            //
            // According to getline()'s documentation, we need to free the line buffer even on
            // failure since we passed NULL for *lineptr.
            free(line);
            break;
        }

        // Effectively remove the trailing newline from the line buffer by overwriting it with a
        // null terminator
        line[strcspn(line, "\n")] = 0;

        // Parse our line into an array of commands
        char** commands = NULL;
        ssize_t command_count = parse_commands(line, &commands);
        if (command_count == -1) {
            // Parsing failed, so bail
            free(line);
            break;
        }

        int ret = run_commands(commands, command_count);
        if (ret == -1) {
            perror("system");
        } else if (ret == 1) {
            // The exit command was entered, so we should exit the shell
            free(commands);
            free(line);
            break;
        }

        free(commands);
        free(line);
    }
}

// Executes the specified batch script
//
// This function takes the provided batch script and executes it line-by-line. If any of the
// commands fails with an error, it returns -1. If the script invoked the "exit" built-in command,
// this function returns 1. Otherwise, it returns 0.
static int run_batch_script(FILE* restrict batch_script) {
    char* line = NULL;
    size_t line_len = 0;
    ssize_t read;

    // Read the batch script line by line
    while ((read = getline(&line, &line_len, batch_script)) != -1) {
        // Parse our line into an array of commands
        char** commands = NULL;
        ssize_t command_count = parse_commands(line, &commands);
        if (command_count == -1) {
            // Parsing failed, so bail
            free(line);
            return -1;
        }

        // Run the commands we parsed
        int ret = run_commands(commands, command_count);
        if (ret != 0) {
            // The commands exited with a special error code, so propagate it upward
            free(commands);
            free(line);
            return ret;
        }

        free(commands);
    }

    free(line);

    return 0;
}

// Parses a string into a list of commands
//
// This function parses a string into a list of commands separated by semicolons. Each command is
// represented by a string containing the command and any space-separated arguments passed to it.
//
// On success, this function returns the number of commands read, and `commands` is populated with a
// newly-allocated array of commands. It is the responsibility of the caller to free this array.
//
// On failure, this function returns -1. This occurs when memory allocation for `commands` fails.
// The caller must not free `commands` in this case.
static ssize_t parse_commands(char* restrict str, char*** restrict commands) {
    // The initial number of elements to allocate for our dynamically-sized array
    const size_t DEFAULT_INIT_ALLOC_ITEMS = 4;

    // Allocate an array of char* for us to store our command pointers
    *commands = malloc(sizeof(char*) * DEFAULT_INIT_ALLOC_ITEMS);
    if (*commands == NULL) {
        // malloc failed. Give up.
        return -1;
    }
    ssize_t command_count = 0;
    size_t commands_capacity = DEFAULT_INIT_ALLOC_ITEMS;

    char* command = strtok(str, ";");
    while (command != NULL) {
        // Resize `commands` if necessary
        if ((size_t)command_count == commands_capacity) {
            // We've reached the capacity of our array, so double its size before adding the new
            // command
            *commands = realloc(*commands, sizeof(char*) * commands_capacity * 2);
            if (*commands == NULL) {
                free(*commands);
                return -1;
            }
            // Update capacity to reflect the new array capacity
            commands_capacity *= 2;
        }

        // Add command to array
        (*commands)[command_count] = command;
        command_count += 1;

        // Read the next command
        command = strtok(NULL, ";");
    }

    return command_count;
}

// Executes command strings using system()
//
// Iterates over an array of commands, executing each with system(). Checks the return
// status of system() for errors, and returns -1 immediately if an error occurs.
//
// If the "exit" command is encountered, this function returns 1, indicating that the shell should
// exit.
//
// Parameters:
//   commands: Array of command strings.
//   command_count: Number of commands.
//
static int run_commands(char** commands, ssize_t command_count) {
    for (ssize_t i = 0; i < command_count; ++i) {
        // Trim leading whitespace from command
        char* cmd = commands[i];
        while (isspace((unsigned char)*cmd)) cmd++;

        // Find the end of the command string and trim trailing spaces by null-terminating the string
        char* end = cmd + strlen(cmd) - 1;
        while (end > cmd && isspace((unsigned char)*end)) {
            *end = '\0';
            end--;
        }

        // Check if the user entered the "exit" command
        if (strcmp(cmd, "exit") == 0) {
            return 1;
        } else if (system(cmd) == -1) {
            return -1;
        }
    }

    return 0;
}