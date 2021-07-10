
#include <stdio.h>
#include <string.h>
#include <stdlib.h>   // exit()
#include <unistd.h>   // fork(), getpid(), exec()
#include <sys/wait.h> // wait()
#include <signal.h>   // signal()
#include <fcntl.h>    // close(), open()
#include <limits.h>
#define MAX_SIZE 2048
#define STRING_SIZE_BUFFER 100
#define SEPERATOR_DELIMMITER " \t\r\n\a"
char *parse_string_input()
{
    int size_of_buffer = MAX_SIZE;
    char *string_input = (char *)malloc(sizeof(char) * size_of_buffer);
    int character;
    if (!string_input)
    {
        fprintf(stderr, "Ne memory\n");
        exit(EXIT_FAILURE);
    }
    /// taking input from user
    int last = 0;
    while (1)
    {
        character = getchar();
        // here is the interesting thing we will take input till we hit null/EOF or \n
        // as size we can't predict we need to reallocate as the size grows
        if (character == EOF || character == '\n')
        {
            string_input[last] = '\0';
            return string_input;
        }
        else
        {
            string_input[last] = character;
        }
        last += 1;
        // if we exceed the size last time allocated then we need to reallocate
        if (last >= size_of_buffer)
        {
            size_of_buffer = 2 * size_of_buffer;
            // allocating twice the initial allocated
            string_input = realloc(string_input, size_of_buffer);
            if (!size_of_buffer)
            {
                fprintf(stderr, "Not enough memory try after some time\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}
int parseInput(char *string)
{
    // This function will parse the input string into multiple commands or a single command with arguments depend_of_stringing on the delimiter (&&, ##, >, or spaces).
    int return_value = -1;
    if (strcmp(string, "exit") == 0)
    {
        // as soon as we get the "exit we terminate our shell"
        return_value = 0;
    }
    else if (strstr(string, "&&"))
    {
        return_value = 1;
    }
    else if (strstr(string, "##"))
    {
        return_value = 2;
    }
    else if (strstr(string, ">"))
    {
        return_value = 3;
    }
    else
    {
        return_value = 4; //for single command
    }
    return return_value;
}
char **spaceRemover(char *string)
{
    int buffer_size = STRING_SIZE_BUFFER, index_position = 0;
    char **return_commands_array = malloc(buffer_size * sizeof(char *));
    // this will represen the argument list to pass to funtion execvp()
    char *temp_checker;

    if (!return_commands_array)
    {
        fprintf(stderr, "Error:Not Enough Memory\n");
        exit(EXIT_FAILURE);
    }
    temp_checker = strtok(string, SEPERATOR_DELIMMITER);
    while (temp_checker != NULL)
    {
        return_commands_array[index_position] = temp_checker;
        index_position++;

        if (index_position >= buffer_size)
        {
            buffer_size += STRING_SIZE_BUFFER;
            return_commands_array = realloc(return_commands_array, buffer_size * sizeof(char *));
            if (!return_commands_array)
            {
                fprintf(stderr, "Error:Not Enough Memory\n");
                exit(EXIT_FAILURE);
            }
        }

        temp_checker = strtok(NULL, SEPERATOR_DELIMMITER);
    }
    return_commands_array[index_position] = NULL;
    return return_commands_array;
}
void executeCommand(char *string)
{
    // This function will fork a new process to execute a command
    // Forking a child
    char **command = spaceRemover(string);
    int flag = 0;
    if (strstr(string, "cd") != NULL)
    {
        int flag = 1;
        if (strstr(string, "cd..") != NULL)
            chdir("..");
        else
            chdir(command[1]);
        return;
    }
    char *arguments_args[] = {command[0], command[1], NULL};
    pid_t process_id = fork();
    if (process_id == -1)
    {
        printf("\nFailed forking child..");
        return;
    }
    else if (process_id == 0)
    {
        //child process running
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        if (execvp(arguments_args[0], arguments_args) < 0)
        {
            printf("Shell: Incorrect command\n");
        }
        exit(0);
    }
    else
    {
        // wait until child gets terminate
        wait(NULL);
        return;
    }
}

void executeParallelCommands(char *string)
{
    // This function will run multiple commands in parallel
    // This function will run multiple commands in parallel
    char *command[3];
    // this will be args
    char *end_of_string, *temp_checker;
    // duplicate string to r
    end_of_string = strdup(string);
    while ((temp_checker = strsep(&end_of_string, "&&")) != NULL)
    {
        command[0] = strdup(temp_checker);
        char **arguments_args = spaceRemover(command[0]);
        pid_t processId = fork();
        if (processId == -1)
        {
            printf("\nFailed forking child..");
            return;
        }
        else if (processId == 0)
        {
            //in child process
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            if (execvp(arguments_args[0], arguments_args) < 0)
            {
                printf("Shell: Incorrect command\n");
            }
            exit(0);
        }
        else
        {
            // do nothing as
            // we are going through parrallel commands
        }
    }
    return;
}

void executeSequentialCommands(char *string)
{
    // This function will run multiple commands in parallel
    // This function will run multiple commands in parallel
    char *end_of_string, *temp_checker;
    end_of_string = strdup(string);
    while ((temp_checker = strsep(&end_of_string, "##")) != NULL)
    {
        if (strstr(temp_checker, "cd") != NULL)
        {
            executeCommand(temp_checker);
            continue;
        }
        char **arguments_args = spaceRemover(strdup(temp_checker));
        pid_t processId = fork();
        if (processId == -1)
        {
            printf("\nFailed forking child..");
            return;
        }
        else if (processId == 0)
        {
            if (execvp(arguments_args[0], arguments_args) < 0)
            {
                printf("Shell: Incorrect command\n");
            }
            exit(1);
        }
        else
        {
            wait(NULL);
        }
    }
}

void executeCommandRedirection(char **string)
{
    // This function will run a single command with output redirected to an output file specificed by user

    // This function will run a single command with output redirected to an output file specificed by user
    // printf("%s\n", temp);
    // 	printf("The command is %s,the file:%s",command[0],command[1]);
    char **name_of_file = spaceRemover(string[1]);
    pid_t processId = fork();
    if (processId < 0) // fork() failed need to exit()
    {
        exit(0);
    }
    if (processId == 0) //child process
    {
        close(STDOUT_FILENO);
        //close the stdout
        open(name_of_file[0], O_CREAT | O_RDWR, S_IRWXU);
        //open file where output going to stay
        // char **cmd_arguments_args = parse_single_command(cmd);
        char **arguments_args = spaceRemover(string[0]);
        int return_value = execvp(arguments_args[0], arguments_args);
        if (return_value < 0)
        { //execvp error code
            printf("Shell: Incorrect command\n");
            exit(1);
        }
    }
    else //parent process
    {
        wait(NULL); //wait till child terminates
    }
}

int main()
{
    signal(SIGINT, SIG_IGN);  // Ignoring SIGINT signal
    signal(SIGTSTP, SIG_IGN); //disable signals so shell can  terminate by exit
    while (1)                 // This loop will keep your shell running until user exits.
    {
        char current_working_directory[PATH_MAX];
        if (getcwd(current_working_directory, sizeof(current_working_directory)) != NULL)
        {
            printf("%s$", current_working_directory);
        }
        else
        {
            perror("Problem with getting current working directory\n");
            return 1;
        }
        char *string_commands = parse_string_input();
        int t = parseInput(string_commands);
        if (t == 0)
        {
            printf("Exiting shell...\n");
            break;
        }
        else
        {
            char *command[3];
            char *end_of_string, *temp_checker;
            end_of_string = strdup(string_commands);
            int i = 0;
            switch (t)
            {
            case 1:
                // This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
                executeParallelCommands(string_commands);
                break;
            case 2:
                // This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
                executeSequentialCommands(string_commands);
                break;
            case 3:
                // This function is invoked when user wants redirect output of a single command to and output file specificed by user
                while ((temp_checker = strsep(&end_of_string, ">")) != NULL)
                {
                    command[i] = strdup(temp_checker);
                    i += 1;
                }
                executeCommandRedirection(command);
                break;
            case 4:
                // for single commands
                executeCommand(string_commands);
                break;
            }
        }
    }

    return 0;
}
