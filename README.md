# Command Line Shell README

This command line shell program provides an interactive prompt for users to enter commands or executes a batch script specified as a command line argument. It supports parsing multiple commands separated by semicolons and executing them using the `system()` function. Additionally, it recognizes the "exit" command to terminate the shell.

## Features

- **Interactive Prompt**: Users can enter commands interactively at the prompt.
- **Batch Script Execution**: Users can specify a batch script as a command line argument to execute multiple commands non-interactively.
- **Command Parsing**: Commands are parsed from input lines or batch script lines, allowing for multiple commands separated by semicolons.
- **Command Execution**: Commands are executed using the `system()` function, providing access to system commands and utilities.
- **Error Handling**: Error messages are provided for file operations, command execution failures, and memory allocation errors.

## Usage

### Interactive Mode

To run the shell in interactive mode, simply execute the program without any command line arguments:

```bash
./shell
