#include "quash.h" //Importing header files needed to run program


void quash_pwd(char **args){
    if(args[1] == NULL){
        printf("PWD enviroment variable not found.");
    }else{
        printf("%s", getenv("PWD"));
    }
}

//Func that terminates the Quash Shell when "exit" or "quit" is typed in the command line
void quash_exit(char **args){
    exit(0);
}

//Function that handles the "cd" command
void quash_cd(char **args){
    if (args[1] == NULL){
        fprintf(stderr, "Quash: cd: missing arguments\n");
    }else{
        if(chdir(args[1]) != 0){
            perror("quash: cd");
        }
    }
}

//Registering the association between a command name and its handler function
struct builtin
{
    //Creating a struct
    char *name;
    void(*func)(char **args);
};

struct builtin builtins[] = {
    //Creating an array of structs
    //If the command name is "exit" the quash exit function runs
    //Same for other associations
    {"exit", quash_exit},
    {"quit", quash_exit},
    {"cd", quash_cd},
    {"echo $PWD", quash_pwd},
};

int quash_num_builtins(){
    return sizeof(builtins) / sizeof(struct builtin);
}

void quash_exec(char **args){

    //Checks for a builtin command before launching an external process
    for (int i = 0; i < quash_num_builtins(); i++){
        if(strcmp(args[0], builtins[i].name) == 0){
            builtins[i].func(args);
            return;
        }
    }

    pid_t child_pid = fork(); 

    if(child_pid == 0){
        //If fork returns 0, we are in the child process
        execvp(args[0], args);
        //Need to use exec to load our target program, execvp will search the directories specified in the $PATH
        //enviroment variable for the executable file named in its first argument.
        //The second argument is a NULL terminated array of string points specifiying arguments for the executable 
        perror("quash");
        exit(1);
    }else if(child_pid > 0){
        //We are in parent process, the return value is the process ID of the child we just launched.
        //We will wait for the child to finish its work and exit before continuing.
        int status;
        do{
            waitpid(child_pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }else{
        //If fork returns a negative value, then an error occured.
        perror("quash");
    }
}

char * quash_read_line(){
    /*
    Using the getline() function from the <stdio.h> to read a single line of input from the stdin
    */

    char *line = NULL;
    size_t buflen = 0;
    getline(&line, &buflen, stdin);
    return line;

    /*
    We'll need to free the memory allocated by the getline() call 
    before we loop to the next command or we'll end up with a memory leak.

    First, split the input string into an array of individual tokens.
    */
}

char** quash_split_line(char* line){
    /*
    We use the strtok() function from <string.h> to split the string on instances of 
    whitespace. Leading and trailing whitespace is ignored and consecutive 
    whitespace characters are treated as a single delimiter.
    */
    int length = 0;
    int capacity = 16;
    char **tokens = malloc(capacity * sizeof(char*));

    char *delimiters = " \t\r\n";
    char *token = strtok(line, delimiters);

    while (token != NULL) {
        tokens[length] = token;
        length++;

        if (length >= capacity) {
            capacity = (int) (capacity * 1.5);
            tokens = realloc(tokens, capacity * sizeof(char*));
        }

        token = strtok(NULL, delimiters);
    }

    tokens[length] = NULL;
    return tokens;

    /*The return value of kash_split_line() is a NULL terminated array of string pointers 
    (i.e. the final value in the array is always NULL). 
    We'll need to free the memory occupied by this array 
    when we're finished with it.
    */
}

int main(){

    /*
    A shell is a command processor. Our shell needs to read in commands from the user,
    interpret them, and act on them — and it needs to keep on doing so, 
    command after command after command.
    */

   //This requires an infinite loop
    while(true){

        /*
        Inside this loop we:
        1. Display a prompt and read in a single line of input from the user
        2. Split the input into an array of tokens which we interpret as a command and its arguments
        3. Execute the command
        4. Free up allocated memory before looping to read in the next command
        */

        printf("[QUASH]$ ");
        char *line = quash_read_line();
        char **tokens = quash_split_line(line);

        if(tokens[0] != NULL){
            quash_exec(tokens);
        }

        free(tokens);
        free(line);
    }
}
