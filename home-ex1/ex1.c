// ========= general_includes.h =========
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>

#define MEM_ALLOC_ERR 12



// ========= global_variables.h =========

// #include "general_includes.h"

#define DEFAULT_MAX_GLOBAL_VARIABLES 20 // the default maximum amount of global variables.

// define a struct that saves a variable name and it value for global variables.
typedef struct {
    char* name;
    char* value;
} Variable;

// define a struct that saves the current global variables, and the current command / argument count.
typedef struct {
    Variable* variables;
    int amount;
    int max;
    int successCommand;
    int successArgument;
} GlobalVariables;

/**
 * @brief Initialize a new GlobalVariables instance.
 *
 * @param vars The instance of GlobalVariables to initialize.
 * @return int 0 if successful, or error code otherwise.
 */
int initGlobalVars(GlobalVariables* vars);

/**
 * @brief Free all memory allocated in an instance of GlobalVariables.
 *
 * @param vars The instance of GlobalVariables to free.
 * @return int 0 if successful, or error code otherwise.
 */
int freeGlobalVars(GlobalVariables* vars);

/**
 * @brief Add another global variable to an instance of GlobalVariables.
 *
 * @param vars The instance of GlobalVariables to add.
 * @param name The name of the variable.
 * @param value The value of the variable.
 * @return int 0 if successful, or error code otherwise.
 */
int addGlobalVar(GlobalVariables* vars, char* name, char* value);

/**
 * @brief Get a global variable from an instance of GlobalVariables by name, and save it to target.
 *
 * @param vars The instance of GlobalVariables to search in.
 * @param name The name of the variable.
 * @param target The target string to copy to (assuming it is big enough).
 */
void getGlobalVar(GlobalVariables* vars, char* name, char* target);


/**
 * @brief Add a command and the amount of arguments it has to the total saved in a GlobalVariables instance.
 *
 * @param vars The instance of GlobalVariables to add the amounts to.
 * @param args The command and its arguments.
 */
void addArgsAmount(GlobalVariables* vars, char* args[]);

/**
 * @brief When a fatal error occurs, such as a memory allocation error
 * call this function to immediatly free all memory allocated in a
 * GlobalVariables instance, and exit the program.
 *
 * @param vars The GlobalVariables instance to free.
 */
void fatalErrorExit(GlobalVariables* vars);



// ========= global_variables.c =========

// #include "global_variables.h"

int initGlobalVars(GlobalVariables* vars) {
    vars->variables = calloc(sizeof(Variable*), DEFAULT_MAX_GLOBAL_VARIABLES);
    vars->max = DEFAULT_MAX_GLOBAL_VARIABLES;

    vars->amount = 0;
    vars->successArgument = 0;
    vars->successCommand = 0;
}

int freeGlobalVars(GlobalVariables* vars) {
    for (int i = 0; i < vars->amount; i++) {
        free(vars->variables[i].name);
    }
    free(vars->variables);
}

int addGlobalVar(GlobalVariables* vars, char* name, char* value) {
    for (int i = 0; i < vars->amount; i++) {
        if (strcmp(vars->variables[i].name, name) == 0) {
            if (strlen(value) > strlen(vars->variables[i].value)) {
                int newLen = strlen(name) + strlen(value) + 2;
                char* temp = realloc(vars->variables[i].name, newLen);

                if (temp == NULL)
                    goto MEMORY_ALLOC_ERR;

                vars->variables[i].name = temp;
                vars->variables[i].value = temp + strlen(name) + 2;
            }
            strcpy(vars->variables[i].value, value);
        }
    }

    if (vars->amount >= vars->max) {
        vars->max *= 2;
        Variable* temp = realloc(vars->variables, sizeof(Variable*) * vars->max);

        if (temp == NULL) {
            vars->max /= 2;
            goto MEMORY_ALLOC_ERR;
        }

        vars->variables = temp;
    }

    char* newName = calloc(sizeof(char), strlen(name) + strlen(value) + 2);
    if (newName == NULL) {
        goto MEMORY_ALLOC_ERR;
    }
    char* newValue = newName + strlen(name) + 2;

    strcpy(newName, name);
    strcpy(newValue, value);

    vars->variables[vars->amount].name = newName;
    vars->variables[vars->amount].value = newValue;
    vars->amount++;
    return 0;

MEMORY_ALLOC_ERR:
    perror("ERR\n");
    return MEM_ALLOC_ERR;
}

void fatalErrorExit(GlobalVariables* vars) {
    freeGlobalVars(vars);
    exit(1);
}

void getGlobalVar(GlobalVariables* vars, char* name, char* target) {
    for (int i = 0; i < vars->amount; i++) {
        if (strcmp(vars->variables[i].name, name) == 0) {
            strcpy(target, vars->variables[i].value);
            return;
        }
    }
    target[0] = '\0';
}

void addArgsAmount(GlobalVariables* vars, char* args[]) {
    int count = 0;
    while (args[++count] != NULL);
    vars->successArgument += count - 1;
    vars->successCommand++;
}



// ========= commands.h =========

// #include "global_vars.h"
#define MAX_INPUT_LEN 511           // Maximum input length (511 to include 510 characters and a '\0' at the end)
#define MAX_ARG_LEN 11              // Maximum argument amount (11 to include 10 arguments and a NULL at the end)

#define VAR_MARKERS "$"             // Tokens to find variables in a string.
#define VAR_SEPERATORS "="          // Tokens to find variable assignment operation in a string.

#define COMMAND_SEPERATORS ";"      // Tokens to find different commands in a string.
#define ARGUMENT_SEPERATORS " \""   // Tokens to find different arguments in a string.
#define ARGUMENT_QUOTES "\""        // Tokens to find quoted parts inside a string.

#define ERR_TOO_MANY_SPLITS 1       // error code for when strSplit tried exceeded resLen.

/**
 * @brief Get a string representing multiple commands, seperated by COMMAND_SEPERATORS tokens.
 * Split the string to multiple strings each representing one command.
 * Will ignore anything in between ARGUMENT_QUOTES tokens.
 *
 * @param str The tring we want to split.
 * @param res The result array of pointers that holds pointers to all parts of the string.
 * @param resLen The length of the res array (including a NULL in the end).
 * @param vars The current global variables instance of the shell (for the case of a fatal error exit).
 * @return int 0 if successful, error code otherwise.
 */
int splitCommands(char str[], char* res[], int resLen, GlobalVariables* vars);

/**
 * @brief Get a string representing a command, seperated by ARGUMENT_SEPERATORS tokens.
 * Split the string to multiple strings each representing one argument.
 * Will ignore anything in between ARGUMENT_QUOTES tokens.
 *
 * @param str The tring we want to split.
 * @param res The result array of pointers that holds pointers to all parts of the string.
 * @param resLen The length of the res array (including a NULL in the end).
 * @param vars The current global variables instance of the shell (for the case of a fatal error exit).
 * @return int 0 if successful, error code otherwise.
 */
int splitArguments(char str[], char* res[], int resLen, GlobalVariables* vars);

/**
 * @brief Get a string and split it to substrings based on seperate tokens.
 * Also ignore seperate tokens that exist in between ignore tokens (to handle cases where quotes exist).
 * Adding a NULL pointer after the last pointer in the res array.
 *
 * @param str The tring we want to split.
 * @param seperateTokens Tokens to seperate the string.
 * @param quotesTokens Tokens used to find places in the string to ignore.
 * @param res The result array of pointers that holds pointers to all parts of the string.
 * @param resLen The length of the res array (including a NULL in the end).
 * @return int 0 if successful, error code otherwise.
 */
int splitStrIgnoreQuotes(char str[], char seperateTokens[], char quotesTokens[], char* res[], int resLen);


/**
 * @brief Recieves an args array representing either a command, or a variable addition operation.
 * Classifies which type of operation to do, and calls the corresponding function to execute it.
 *
 * @param args argumens array representing either a command.
 * @param vars The current instance of GlobalVariables.
 * @return int 0 if successful, error code otherwise.
 */
int executeOperation(char* args[], GlobalVariables* vars);

/**
 * @brief Fork this program and call the execvp function using the args array recieved.
 * If successful, will wait for the call of execvp to end then return 0.
 * If not successful, will return error code.
 *
 * @param args Arguments array for execvp function.
 * @return int 0 if successful, error code otherwise.
 */
int callExecvp(char* args[]);

/**
 * @brief Recieve an argument array args where the first argument of the form
 * "<varuable name>=<value>" OR "<varuable name>="
 * Will assign the variable name the value in the GlobalVariables instance.
 * Supports multiple variables in a row (multiple arguments of the form above).
 *
 * @param args The arguments that hold variable names and values.
 * @param vars The current instance of GlobalVariables.
 * @return int 0 if successful, error code otherwise.
 */
int callVariableAssign(char* args[], GlobalVariables* vars);

/**
 * @brief Recieve the name of a command.
 * Return true if the command is supported, false otherwise.
 *
 * @param commandName The command name
 * @return true If the command is supported.
 * @return false If the command is not supported.
 */
bool isSupportedCommand(char commandName[]);

/**
 * @brief Recieves a string.
 * Replaces every instance of "$<variable name>" (where $ may be any element of VAR_MARKERS)
 * inside the string with the corresponding "<value>" withing vars.
 * If there is no such <value>, puts an empty string instead.
 *
 * @param str The string in which we want to replaces the variable names.
 * @param vars The current instance of GlobalVariables.
 * @return int 0 if successful, error code otherwise.
 */
int replaceVars(char* str, GlobalVariables* vars);



// ========= commands.c =========

// #include "commands.h"

int splitStrIgnoreQuotes(char str[], char seperateTokens[], char quotesTokens[], char* res[], int resLen) {
    char* tokens = calloc(sizeof(char), strlen(seperateTokens) + strlen(quotesTokens) + 1);
    if (tokens == NULL)
        return MEM_ALLOC_ERR;

    strcpy(tokens, seperateTokens);
    strcat(tokens, quotesTokens);

    char* p = strpbrk(str, tokens);
    char temp;

    int index = 0;
    res[0] = str;

    bool isInQuotes = false;
    while (p != NULL) {
        temp = *p;
        if (strchr(quotesTokens, temp)) {
            isInQuotes = !isInQuotes;
        }
        if (strchr(seperateTokens, temp)) {
            *p = '\0';
            if (*(res[index]) != '\0' && ++index >= resLen - 1)
                goto TOO_MANY_ARGS;

            res[index] = p + 1;
        }
        if (isInQuotes)
            p = strchr(p + 1, temp);
        else
            p = strpbrk(p + 1, tokens);
    }
    res[++index] = NULL;
    free(tokens);
    return 0;

TOO_MANY_ARGS:
    free(tokens);
    return ERR_TOO_MANY_SPLITS;
}

int executeOperation(char* args[], GlobalVariables* vars) {
    int err = 0;
    char* equals = strpbrk(args[0], VAR_SEPERATORS);

    if (equals != NULL) {
        err = callVariableAssign(args, vars);
    }
    else if (isSupportedCommand(args[0])) {
        err = callExecvp(args);
        if (err) {
            freeGlobalVars(vars);
            exit(1);
        }
    }
    else {
        fprintf(stderr, "%s unsupported command", args[0]);
        return 1;
    }

    if (err == 0)
        addArgsAmount(vars, args);

    return err;
}

int callVariableAssign(char* args[], GlobalVariables* vars) {
    char* name, * value, * equals;

    int err = 0;
    for (int i = 0; args[i] != NULL; i++) {
        equals = strpbrk(args[i], VAR_SEPERATORS);
        if (equals == NULL)
            break;
        *equals = '\0';
        name = args[i];
        value = (equals + 1);
        if (*value == '\0' && args[i + 1] != NULL)
            value = args[++i];

        err = addGlobalVar(vars, name, value);
        if (err != 0)
            break;
    }
    return err;
}

int callExecvp(char* args[]) {
    int sonId = fork();
    if (sonId == 0) {
        execvp(args[0], args);
        fprintf(stderr, "%s", args[0]);
        perror("ERR\n");
        exit(1);
    }
    if (sonId > 0) {
        wait(NULL);
        return 0;
    }
    perror("ERR\n");
    return 1;
}

bool isSupportedCommand(char commandName[]) {
    if (strcmp(commandName, "cd") == 0)
        return false;
    return true;
}

void requestInput(char str[], int n) {
    str[0] = '\0';
    char* res = fgets(str, n, stdin);
    str[strlen(str) - 1] = '\0';
}

int replaceVars(char* str, GlobalVariables* vars) {
    char* end = NULL;
    char temp[MAX_INPUT_LEN] = "";
    char value[MAX_INPUT_LEN] = "";

    char* start = strpbrk(str, VAR_MARKERS);

    while (start != NULL) {
        *start = '\0';
        end = strchr(start + 1, ' ');
        if (end != NULL) {
            strcpy(temp, end);
            *end = '\0';
        }
        else
            temp[0] = '\0';

        getGlobalVar(vars, start + 1, value);
        if (strlen(start) + strlen(value) + strlen(temp) + 1 >= MAX_INPUT_LEN) {
            goto INPUT_TOO_LONG;
        }
        strcat(str, value);
        strcat(str, temp);
        start = strpbrk(start + strlen(value), VAR_MARKERS);
    }
    return 0;

INPUT_TOO_LONG:
    fprintf(stderr, "Input too long");
    return 1;
}

int splitCommands(char str[], char* res[], int resLen, GlobalVariables* vars) {
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        switch (str[i]) {
        case ';':
            count++;
            break;
        case ' ':
            continue;
        default:
            count = 0;
        }
        if (count >= 2) {
            fprintf(stderr, "Invalid input - unexpected token ;\n");
            return 1;
        }
    }
    int err = splitStrIgnoreQuotes(str, COMMAND_SEPERATORS, ARGUMENT_QUOTES, res, resLen);
    if (err == ERR_TOO_MANY_SPLITS) {
        fprintf(stderr, "Inavlid input - too many commands\n");
    }
    if (err == MEM_ALLOC_ERR)
        fatalErrorExit(vars);
    return err;
}

int splitArguments(char str[], char* res[], int resLen, GlobalVariables* vars) {
    int err = splitStrIgnoreQuotes(str, ARGUMENT_SEPERATORS, ARGUMENT_QUOTES, res, resLen);
    if (err == ERR_TOO_MANY_SPLITS) {
        fprintf(stderr, "Inavlid input - too many arguments\n");
    }
    if (err == MEM_ALLOC_ERR)
        fatalErrorExit(vars);
    return err;
}

// --- main.h ---

//#include global_vars.h
//#include string_funcs.h
//#include commands.h

#define MAX_ENTER_PRESS 3

/**
 * @brief This function runs a shell.
 * Pressing enter 3 times will exit the shell.
 *
 * @return int 0 if existed normally, error code otherwise.
 */
int runShell();

// --- main.c ---

int runShell() {
    // Caculate maximum possible commands
    const int MAX_POSSIBLE_COMMANDS = MAX_INPUT_LEN / 2 + 1;

    // Initialize GlobalVariables
    GlobalVariables vars;
    initGlobalVars(&vars);

    // Initialize input and commands.
    char strInput[MAX_INPUT_LEN];
    char* commands[MAX_POSSIBLE_COMMANDS];
    char* args[MAX_ARG_LEN];
    char* check = NULL;
    int counter = 0;

    char currentDir[MAX_INPUT_LEN] = "";
    getcwd(currentDir, MAX_INPUT_LEN);

    int err = 0;
    while (counter < MAX_ENTER_PRESS) {
        printf("cmd:%d|#args:%d@%s>", vars.successCommand, vars.successArgument, currentDir);
        requestInput(strInput, MAX_INPUT_LEN);

        if (strInput[0] == '\0') {
            counter++;
            continue;
        }
        else
            counter = 0;

        replaceVars(strInput, &vars);

        if (splitCommands(strInput, commands, MAX_POSSIBLE_COMMANDS, &vars))
            continue;

        for (int i = 0; commands[i] != NULL; i++) {
            if (splitArguments(commands[i], args, MAX_ARG_LEN, &vars))
                continue;

            if (executeOperation(args, &vars) == MEM_ALLOC_ERR)
                goto MEMORY_ALLOC_ERR;
        }
    }

    freeGlobalVars(&vars);
    return 0;

MEMORY_ALLOC_ERR:
    perror("ERR");
    return MEM_ALLOC_ERR;
}

int main() {
    return runShell();
}