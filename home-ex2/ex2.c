// ========= general_includes.h =========
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MEM_ALLOC_ERR 12

// ========= global_variables.h =========

// #include "general_includes.h"
#define DEFAULT_MAX_GLOBAL_VARIABLES 20 // the default maximum amount of global variables.

/**
 * @brief define a struct that saves a variable name and it value for global variables.
 *
 */
typedef struct {
    char* name;
    char* value;
} Variable;

/**
 * @brief define a struct that saves the current global variables, and the current command / argument count.
 *
 */
typedef struct {
    Variable* variables;
    int amount;
    int max;
    int successCommand;
    int successArgument;
} GlobalVariables;

/**
 * @brief initialize current global variables.
 *
 * @return int 0 if successful, error code otherwise.
 */
int initGlobalVars();

/**
 * @brief free current global variables.
 *
 * @return int 0 if successful, error code otherwise.
 */
int freeGlobalVars();

/**
 * @brief save a new global variable.
 *
 * @param name the name of the global variable.
 * @param value the value of the global varialbe.
 * @return int
 */
int addGlobalVar(char* name, char* value);


/**
 * @brief save the value of gloabal variable <name> in target.
 *
 * @param name
 * @param target
 */
void getGlobalVar(char* name, char* target);

/**
 * @brief update the amount of successful commands and arguments in the current global variables according to args.
 *
 * @param args the arguments of a new command to add to the count.
 */
void updateArgsAmount(char* args[]);

/**
 * @brief Get the current Command Count.
 *
 * @return int the current command count.
 */
int getCommandCount();

/**
 * @brief Get the current Argument Count
 *
 * @return int - the current argument count.
 */
int getArgumentCount();

/**
 * @brief free all global variable resources and exit.
 *
 */
void fatalErrorExit(const char* err);


// ========= global_variables.c =========

// #include "global_variables.h"

// save a "singleton" instance of currentGlobalVariables.
GlobalVariables* currentGlobalVariables = NULL;

int initGlobalVars() {
    if (currentGlobalVariables != NULL) {
        return MEM_ALLOC_ERR;
    }

    currentGlobalVariables = calloc(sizeof(GlobalVariables), 1);
    GlobalVariables* vars = currentGlobalVariables;
    vars->variables = calloc(sizeof(Variable*), DEFAULT_MAX_GLOBAL_VARIABLES);
    vars->max = DEFAULT_MAX_GLOBAL_VARIABLES;

    vars->amount = 0;
    vars->successArgument = 0;
    vars->successCommand = 0;
}

int freeGlobalVars() {
    if (currentGlobalVariables == NULL) {
        return MEM_ALLOC_ERR;
    }
    GlobalVariables* vars = currentGlobalVariables;
    for (int i = 0; i < vars->amount; i++) {
        free(vars->variables[i].name);
    }
    free(vars->variables);
    free(currentGlobalVariables);
    currentGlobalVariables = NULL;
}

int addGlobalVar(char* name, char* value) {
    if (currentGlobalVariables == NULL) {
        return MEM_ALLOC_ERR;
    }

    GlobalVariables* vars = currentGlobalVariables;
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
            return 0;
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
    fatalErrorExit("memory allocation error");
}

void getGlobalVar(char* name, char* target) {
    target[0] = '\0';
    if (currentGlobalVariables == NULL) {
        return;
    }

    GlobalVariables* vars = currentGlobalVariables;
    for (int i = 0; i < vars->amount; i++) {
        if (strcmp(vars->variables[i].name, name) == 0) {
            strcpy(target, vars->variables[i].value);
            return;
        }
    }
}

void updateArgsAmount(char* args[]) {
    GlobalVariables* vars = currentGlobalVariables;
    int count = 0;
    while (args[++count] != NULL);
    vars->successArgument += count - 1;
    vars->successCommand++;
}

int getCommandCount() {
    if (currentGlobalVariables == NULL)
        return -1;
    return currentGlobalVariables->successCommand;
}

int getArgumentCount() {
    if (currentGlobalVariables == NULL)
        return -1;
    return currentGlobalVariables->successArgument;
}

void fatalErrorExit(const char* err) {
    perror(err);
    freeGlobalVars();
    exit(1);
}

// ========= pid_groups.h =========

#define MAX_PID_GROUP_AMOUNT 1000

/**
 * @brief Set the Current foreground group id
 *
 * @param foregroundGroupID
 * @return int
 */
int setForegroundGroupID(int foregroundGroupID);

/**
 * @brief Get the Current foreground group id
 *
 * @return int the current foreground group id, -1 if there is no current FG group id.
 */
int getGoregroundGroupID();

/**
 * @brief clear the curre foreground group id.
 *
 */
void clearCurrentFG();

/**
 * @brief push a paused process group id to the paused stack.
 *
 * @param pausedProcessGroupID the GROUP
 */
void pushPause(int pausedProcessGroupID);

/**
 * @brief pop a paused process group id from the stack and return it.
 *
 * @return int the group id which was popped from the stack.
 */
int popPause();

/**
 * @brief Add a group ID to the list of this process's child group ID's.
 *
 * @param groupID the group ID of several child processes.
 */
void addGroupID(int groupID);

/**
 * @brief remove a group ID from the list.
 *
 * @param groupID the group ID to remove.
 * @return int the group ID that was removed.
 */
int removeGroupID(int groupID);

/**
 * @brief Remove a group ID by index.
 *
 * @param index index of the groupID.
 * @return int the group ID which was removed.
 */
int removeGroupIDbyIndex(int index);

// ========= pid_groups.c =========

int backgroundGroupIDs[MAX_PID_GROUP_AMOUNT];
int backgroundGroupIDAmount;

int pausedGroupIDs[MAX_PID_GROUP_AMOUNT];
int pausedGroupIDAmount;

int currentForegroundProcessID;

int setForegroundGroupID(int foregroundGroupID) {
    currentForegroundProcessID = foregroundGroupID;
}

int getGoregroundGroupID() {
    return currentForegroundProcessID;
}

void clearCurrentFG() {
    currentForegroundProcessID = -1;
}

int initPidGroups() {
    for (int i = 0; i < MAX_PID_GROUP_AMOUNT; i++) {
        backgroundGroupIDs[i] = -1;
        pausedGroupIDAmount = -1;
    }
    backgroundGroupIDAmount = 0;
    pausedGroupIDAmount = 0;
    currentForegroundProcessID = -1;
}

void pushPause(int pausedProcessGroupID) {
    pausedGroupIDs[pausedGroupIDAmount++] = pausedProcessGroupID;
}

int popPause() {
    if (pausedGroupIDAmount == 0)
        return -1;
    return pausedGroupIDs[--pausedGroupIDAmount];
}

void addGroupID(int groupID) {
    backgroundGroupIDs[backgroundGroupIDAmount++] = groupID;
}

int findGroupID(int groupID) {
    for (int i = 0; i < backgroundGroupIDAmount; i++) {
        if (groupID == backgroundGroupIDs[i])
            return i;
    }
    return -1;
}

int removeGroupIDbyIndex(int index) {
    if (index < 0 || index >= backgroundGroupIDAmount)
        return -1;

    int res = backgroundGroupIDs[index];
    for (int i = index; i < backgroundGroupIDAmount; i++) {
        backgroundGroupIDs[i] = backgroundGroupIDs[i + 1];
    }
    backgroundGroupIDs[backgroundGroupIDAmount--] = -1;
    return res;
}

int removeGroupID(int groupID) {
    int i = findGroupID(groupID);
    return removeGroupIDbyIndex(groupID);
}

void printExited() {
    int res = 0;
    int gID = 0;
    int status = 0;
    for (int i = 0; i < backgroundGroupIDAmount; i++) {
        gID = backgroundGroupIDs[i];
        int res = waitpid(-gID, &status, WNOHANG);
        if (res < 0) {
            printf("[%d] done\n", gID);
            removeGroupIDbyIndex(i);
            i--;
        }
    }
}

// ========= string_operations.h =========

#define ERR_TOO_MANY_SPLITS 1       // error code for when strSplit tried exceeded resultArrLength.

/**
 * @brief get the pointer of the next findTokens, ignoring anything between two identical ignore tokens.
 *
 * @param str the string to search in.
 * @param seperateTokens the token to find in the string.
 * @param ignoreTokens the tokens to ignore anything between them.
 * @return char* a pointer to the found char in str, NULL if wasn't found.
 */
char* strpbrkIgnoreQuotes(char str[], char findTokens[], char ignoreTokens[]);

/**
 * @brief split a given string to multiple string for every char in split tokens, ignoring anything between two identical ignore tokens.
 *
 * @param str The string to split.
 * @param splitTokens tokens to find, delete and split the string using.
 * @param ignoreTokens tokens to ignore anything in between (including them).
 * @param resultStrArray the resulting array with all the pointers to the seperated string.
 * @param resultStrArrMaxLength the maximum lendth of resultStrArray.
 * @return int 0 if successful, error code otherwise.
 */
int splitStrIgnoreQuotes(char str[], char splitTokens[], char ignoreTokens[], char* resultStrArray[], int resultStrArrMaxLength);

/**
 * @brief Remove tokens from a string, ignoring anything between two identical ignore tokens.
 *
 * @param str the string to remove tokens from.
 * @param extraTokens the tokens to remove.
 * @param ignoreTokens the tokens to ignore anything between.
 * @return int 0 if successful, error code otherwise.
 */
int removeExtraTokensWhileIgnoring(char str[], char extraTokens[], char ignoreTokens[]);

/**
 * @brief remove tokens, ignoring anything between two identical tokens.
 *
 * @param str the string to remove tokens from.
 * @param quotesTokens the tokens to remove.
 * @return int 0 if successful, error code otherwise.
 */
int removeExtraIgnoreTokens(char str[], char quotesTokens[]);

/**
 * @brief turn all English alphabet characters to lower case.
 *
 * @param str a string to turn lower case.
 */
void toLowerCase(char str[]);

// ========= string_operations.c =========

char* strpbrkIgnoreQuotes(char str[], char findTokens[], char ignoreTokens[]) {
    char* curr = strpbrk(str, findTokens);
    if (curr == NULL)
        return NULL;

    char* ignore1, * ignore2;
    ignore1 = strpbrk(str, ignoreTokens);

    while (curr != NULL) {
        if (ignore1 == NULL || curr < ignore1)
            return curr;

        ignore2 = strchr(ignore1 + 1, *ignore1);
        if (ignore2 == NULL) {
            return NULL;
        }

        while (curr != NULL && curr <= ignore2) {
            curr = strpbrk(curr + 1, findTokens);
        }
        ignore1 = strpbrk(ignore2 + 1, ignoreTokens);
    }
    return NULL;
}

int splitStrIgnoreQuotes(char str[], char splitTokens[], char ignoreTokens[], char* resultStrArray[], int resultStrArrMaxLength) {
    int index = 0;
    resultStrArray[index] = str;
    char* curr = str, * next = NULL;
    char nextToken = '\0';

    curr = strpbrkIgnoreQuotes(curr, splitTokens, ignoreTokens);
    while (curr != NULL) {
        *curr = '\0';
        if (*(resultStrArray[index]) != '\0' && ++index >= resultStrArrMaxLength - 1)
            goto TOO_MANY_ARGS;

        resultStrArray[index] = curr + 1;
        curr = strpbrkIgnoreQuotes(curr + 1, splitTokens, ignoreTokens);
    }

    if (resultStrArray[index] != NULL && resultStrArray[index][0] != '\0')
        index++;
    resultStrArray[index] = NULL;
    return 0;

TOO_MANY_ARGS:
    resultStrArray[0] = NULL;
    return ERR_TOO_MANY_SPLITS;
}

int removeExtraTokensWhileIgnoring(char str[], char extraTokens[], char ignoreTokens[]) {
    char currIgnore = '\"';
    bool ignoring = false;

    int index = 0, offset = 0;
    while (str[index + offset] != '\0') {
        str[index] = str[index + offset];
        if (strchr(ignoreTokens, str[index])) {
            ignoring = !ignoring;
        }

        if (ignoring) {
            index++;
            continue;
        }

        if (strchr(extraTokens, str[index]) || ignoring && currIgnore == str[index]) {
            currIgnore = str[index];
            ignoring = !ignoring;
            offset++;
            continue;
        }
        index++;
    }
    str[index] = '\0';
}

int removeExtraIgnoreTokens(char str[], char quotesTokens[]) {
    char currIgnore = '\"';
    bool ignoring = false;

    int index = 0, offset = 0;
    while (str[index + offset] != '\0') {
        str[index] = str[index + offset];
        if (!ignoring && strchr(quotesTokens, str[index]) || ignoring && currIgnore == str[index]) {
            currIgnore = str[index];
            ignoring = !ignoring;
            offset++;
        }
        else
            index++;
    }
    str[index] = '\0';
}

void toLowerCase(char str[]) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] = str[i] - 'A' + 'a';
    }
}

// ========= commands.h =========

// #include "global_vars.h"
#define MAX_INPUT_LEN 510           // Maximum input length (511 to include 510 characters and a '\0' at the end)
#define MAX_ARG_LEN 10              // Maximum argument amount (11 to include 10 arguments and a NULL at the end)
#define MAX_PIPED_COMMANDS 20       // Maximum amount of commands that can pipe to each other.
#define MAX_CONCURRENT_COMMANDS 100 // Maximum amount of commands that can pipe to each other.

#define VAR_MARKER_TOKENS "$"       // Tokens to find variables in a string.
#define VAR_SEPERATOR_TOKENS "="    // Tokens to find variable assignment operation in a string.

#define FG_COMMAND_SEPERATOR_TOKENS ";"   // Tokens to find different "operations" in a string.
#define BG_COMMAND_SEPERATOR_TOKENS "&"   // Tokens to find different "operations" in a string.
#define PIPE_SEPERATOR_TOKENS "|"         // Tokens to find diffenent commands in string that may be piped one after another. 
#define ARGUMENT_SEPERATOR_TOKENS " "   // Tokens to find different arguments in a string.
#define QUOTES_TOKENS "\""              // Tokens to find quoted parts inside a string.

#define FILE_CAT_SEPERATOR_TOKENS ">"   // Tokens to find concantation to files.
#define SPECIAL_CHARS " \";&>|"     // A list of all special characters in strings.

#define ERR_INPUT_TOO_LONG 2

/**
 * @brief used to catch SIGTSTP signal and send it to the current foreground group processes.
 *
 * @param sig the signal that was caught, should always be SIGTSTP.
 */
void catch_SIGTSTP(int sig);

/**
 * @brief safely close a file descriptor, if accidentally sent stdin, stdout and stderr will not close.
 *
 * @param fd
 * @return int
 */
int safelyCloseFileDscriptor(int fd);

/**
 * @brief Call a command using execvp, where args are the arguments of the command.
 *
 * @param args The arguments of the command.
 * @param inPipe The pipe used as input for the command.
 * @param outPipe The pipe used as output for the command.
 * @param groupid The group id of the command, -1 if no group id is seleceted.
 * @param isBG whether or not the command should run in the bakground.
 * @return int
 */
int callCommandUsingExecvp(char* args[], int inPipe[], int outPipe[], int* groupid, bool isBG);


/**
 * @brief Request input from the user, will be saved in the buffer buff.
 *
 * @param buff buffer where input will be saved.
 * @param n the maximum length of the buffer.
 * @return int 0 if the request was successful, error code otherwise.
 */
int request_input_from_user(char buff[], int n);

/**
 * @brief replace globale variables in the input, marked with VARIABLE_MARKERS, with their value.
 *
 */
int replaceVariablesWithValues(char input[]);

/**
 * @brief add a new global variable to the current global variables.
 *
 * @param varStrs a string representing <name>=<value> of a global variable.
 * @return int 0 if successful, error code otherwise.
 */
int handleAddGlobalVaribales(char* varStrs[]);

/**
 * @brief unpause the top paused process in the stack.
 *
 * @return int
 */
int handleUnpauseToBackground();

/**
 * @brief check if the command is a specifal command the shell needs to handle.
 * For example bg, cd.
 *
 * @param command the command in question.
 * @return int 1 if it was a special case, 0 otherwise.
 */
int handleSpecialCases(char* command[]);

/**
 * @brief recieve a string representing a single command and handle it.
 *
 * @param command a string representing a single command.
 * @param inPipe input pipe.
 * @param outPipe output pipe.
 * @param groupID group id, -1 if no group id.
 * @param isBG should the command run in background or not.
 * @return int 0 if successful, 1 otherwise.
 */
int handleSingleCommand(char command[], int inPipe[], int outPipe[], int* groupID, bool isBG);

/**
 * @brief Recieve an operation that may include multiple commands piping to each other, and handle it.
 * for example: ls -l | wc > file.txt
 *
 * @param op a string representing a single command or multiple piped commands.
 * @param isBG should the operation be run in background.
 * @return int 0 if successful, 1 otherwise.
 */
int handlePipedCommands(char op[], bool isBG);

/**
 * @brief Recieve an entire input line from the user, seperate it to single operations and handle each of them.
 *
 * @param input the input from the user.
 * @return int 0 if successful, 1 otherwise.
 */
int handleWholeInput(char input[]);

// ========= commands.c =========
// #include "commands.h"

void catch_execvpFail(int sig) {
    if (sig == SIGUSR1) {

    }
}

void catch_SIGTSTP(int sig) {
    int gpid = getGoregroundGroupID();
    if (gpid <= 0)
        return;
    kill(-gpid, SIGTSTP);
    pushPause(gpid);
    clearCurrentFG();
    printf("[%d] process paused", gpid);
}

int safelyCloseFileDscriptor(int fd) {
    if (fd <= 0 || fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO)
        return 0;
    return close(fd);
}

int callCommandUsingExecvp(char* args[], int inPipe[], int outPipe[], int* groupid, bool isBG) {
    int sonId = fork();
    if (sonId == 0) {
       // printf("%s : %d\n", args[0], getpid());
        safelyCloseFileDscriptor(inPipe[1]);
        safelyCloseFileDscriptor(outPipe[0]);

        dup2(inPipe[0], STDIN_FILENO);
        dup2(outPipe[1], STDOUT_FILENO);

        execvp(args[0], args);
        perror(args[0]);

        safelyCloseFileDscriptor(inPipe[0]);
        safelyCloseFileDscriptor(inPipe[1]);
        safelyCloseFileDscriptor(outPipe[0]);
        safelyCloseFileDscriptor(outPipe[1]);
        exit(1);
    }

    if (sonId > 0) {
        if (*groupid < 0)
            *groupid = sonId;

        setpgid(sonId, *groupid);
        return 0;
    }
    perror("ERR\n");
    return -1;
}

int request_input_from_user(char buff[], int n) {
    buff[n - 1] = '\0';
    buff[n - 2] = '\n';
    fgets(buff, n + 1, stdin);

    if (buff[n - 2] != '\n') {
        while (buff[n - 2] != '\n') {
            buff[n - 2] = '\n';
            fgets(buff, n + 1, stdin);
        }
        return ERR_INPUT_TOO_LONG;
    }

    buff[strlen(buff) - 1] = '\0';
    return 0;
}

int replaceVariablesWithValues(char input[]) {
    char* end = NULL;
    char temp[MAX_INPUT_LEN + 1] = "";
    char value[MAX_INPUT_LEN] = "";

    char* start = strpbrk(input, VAR_MARKER_TOKENS);

    while (start != NULL) {
        *start = '\0';
        end = strpbrk(start + 1, SPECIAL_CHARS);
        if (end != NULL && *end != '\0') {
            strcpy(temp, end);
            *end = '\0';
        }
        else
            temp[0] = '\0';

        getGlobalVar(start + 1, value);
        if (strlen(start) + strlen(value) + strlen(temp) + 1 >= MAX_INPUT_LEN) {
            goto INPUT_TOO_LONG;
        }
        strcat(input, value);
        strcat(input, temp);
        start = strpbrk(start + strlen(value), VAR_MARKER_TOKENS);
    }
    return 0;

INPUT_TOO_LONG:
    fprintf(stderr, "Input too long");
    return ERR_INPUT_TOO_LONG;
}

int handleAddGlobalVaribales(char* varStrs[]) {
    int i = 0;
    char* name, * value, * brk;

    int err = 0;
    for (i = 0; varStrs[i] != NULL; i++) {
        brk = strpbrkIgnoreQuotes(varStrs[i], VAR_SEPERATOR_TOKENS, QUOTES_TOKENS);
        if (brk == NULL || brk == varStrs[i] || (brk - 1) < varStrs[i] || strchr(SPECIAL_CHARS, *(brk - 1)) != NULL)
            break;
        *brk = '\0';
        name = varStrs[i];
        value = brk + 1;
        if ((err = addGlobalVar(name, value)) != 0)
            return err;
        varStrs[i] = NULL;
    }

    if (i == 0)
        return 0;

    for (int j = 0; varStrs[j] != NULL; j++) {
        varStrs[j] = varStrs[j + i];
    }
    return 0;
}

int handleUnpauseToBackground() {
    int gpid = popPause();
    if (gpid <= 0) {
        printf("no paused operation\n");
        return 1;
    }
    kill(-gpid, SIGCONT);
    printf("[%d] continued in background\n", gpid);
}

int handleSpecialCases(char* command[]) {
    if (strcmp(command[0], "bg") == 0) {
        handleUnpauseToBackground();
        return 1;
    }
    if (strcmp(command[0], "cd") == 0) {
        fprintf(stderr, "%s not supported", command[0]);
        return 1;
    }
    return 0;
}

int handleSingleCommand(char command[], int inPipe[], int outPipe[], int* groupID, bool isBG) {
    char* commandAndFiles[MAX_CONCURRENT_COMMANDS] = { 0 };
    char* finalCommand[MAX_CONCURRENT_COMMANDS] = { 0 };
    char* fileNames[MAX_CONCURRENT_COMMANDS] = { 0 };
    char* temp[MAX_CONCURRENT_COMMANDS] = { 0 };

    int file_fd = 0;
    int err = 0;
    if ((err = splitStrIgnoreQuotes(command, FILE_CAT_SEPERATOR_TOKENS, QUOTES_TOKENS, commandAndFiles, MAX_CONCURRENT_COMMANDS)) != 0) {
        return err;
    }

    if ((err = splitStrIgnoreQuotes(commandAndFiles[0], ARGUMENT_SEPERATOR_TOKENS, QUOTES_TOKENS, finalCommand, MAX_CONCURRENT_COMMANDS)) != 0)
        return err;

    char* swapTemp;
    int index = 0;
    int fileIndex = 0;
    for (index = 0; finalCommand[index] != NULL; index++);

    for (int i = 1; commandAndFiles[i] != NULL; i++) {
        if ((err = splitStrIgnoreQuotes(commandAndFiles[i], ARGUMENT_SEPERATOR_TOKENS, QUOTES_TOKENS, temp, MAX_CONCURRENT_COMMANDS)) != 0)
            return err;

        fileNames[fileIndex++] = temp[0];
        for (int j = 1; temp[j] != NULL; j++) {
            finalCommand[index++] = temp[j];
        }
    }

    int res = handleSpecialCases(finalCommand);
    if (res != 0) {
        return -1;
    }

    handleAddGlobalVaribales(finalCommand);
    if (finalCommand[0] == NULL) {
        return 0;
    }

    for (int i = 0; fileNames[i] != NULL; i++) {
        removeExtraTokensWhileIgnoring(fileNames[i], ARGUMENT_SEPERATOR_TOKENS, QUOTES_TOKENS);
        removeExtraIgnoreTokens(fileNames[i], QUOTES_TOKENS);
        if (file_fd > 0) {
            err = safelyCloseFileDscriptor(file_fd);
            file_fd = 0;
        }
        file_fd = open(fileNames[i], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (file_fd < 0) {
            fatalErrorExit("cannot open pipe");
        }
    }

    if (file_fd > 0) {
        safelyCloseFileDscriptor(outPipe[1]);
        outPipe[1] = file_fd;
        ftruncate(file_fd, 0);
    }


    for (int i = 0; finalCommand[i] != NULL;i++) {
        removeExtraTokensWhileIgnoring(finalCommand[i], ARGUMENT_SEPERATOR_TOKENS, QUOTES_TOKENS);
        removeExtraIgnoreTokens(finalCommand[i], QUOTES_TOKENS);
    }
    if (res == 0) {
        updateArgsAmount(finalCommand);
        res = callCommandUsingExecvp(finalCommand, inPipe, outPipe, groupID, isBG);
    }

    safelyCloseFileDscriptor(file_fd);
    return res;
}

int handlePipedCommands(char op[], bool isBG) {
    int pipes[MAX_CONCURRENT_COMMANDS][2] = { 0 };

    char* operationParts[MAX_CONCURRENT_COMMANDS];
    int err = splitStrIgnoreQuotes(op, PIPE_SEPERATOR_TOKENS, QUOTES_TOKENS, operationParts, MAX_CONCURRENT_COMMANDS);
    if (err != 0) {
        fprintf(stderr, "error - too many commands\n");
        return err;
    }

    int countCommands = 0;
    int groupid = -1;

    int stdinPipe[2] = { STDIN_FILENO, -1 };
    int stdoutPipe[2] = { -1, STDOUT_FILENO };

    int* input, * output;

    for (int i = 0; operationParts[i] != NULL; i++) {
        if (operationParts[i + 1] != NULL) {
            err = pipe(pipes[i]);
            if (err < 0) {
                fatalErrorExit("cannot open pipe");
            }
        }

        input = stdinPipe;
        if (i > 0) {
            input = pipes[i - 1];
        }
        output = stdoutPipe;
        if (operationParts[i + 1] != NULL) {
            output = pipes[i];
        }
        handleSingleCommand(operationParts[i], input, output, &groupid, isBG);
        countCommands++;

        safelyCloseFileDscriptor(input[0]);
        safelyCloseFileDscriptor(input[1]);
    }

    if (groupid == 0)
        return 0;

    if (!isBG) {
        setForegroundGroupID(groupid);
        int status = 0;

        while (waitpid(-groupid, &status, WUNTRACED) > 0) {

            if (!WIFEXITED(status)) {
                clearCurrentFG();
                isBG = !isBG;
                break;
            }
        }
    }
    if (isBG) {
        addGroupID(groupid);
        printf("[%d]\n", groupid);
    }
}

int handleWholeInput(char input[]) {
    char* seperatedTemp[MAX_CONCURRENT_COMMANDS] = { 0 };
    char* seperatedCommands[MAX_CONCURRENT_COMMANDS] = { 0 };
    bool isBG[MAX_CONCURRENT_COMMANDS] = { false };

    int err = splitStrIgnoreQuotes(input, FG_COMMAND_SEPERATOR_TOKENS, QUOTES_TOKENS, seperatedTemp, MAX_CONCURRENT_COMMANDS);
    if (err != 0) {
        fprintf(stderr, "error - too many commands\n");
        return err;
    }

    char* brk;
    char* currStr;
    int index = 0;
    for (int i = 0; seperatedTemp[i] != NULL; i++) {
        currStr = seperatedTemp[i];
        brk = strpbrkIgnoreQuotes(currStr, BG_COMMAND_SEPERATOR_TOKENS, QUOTES_TOKENS);
        while (*currStr != '\0' && brk != NULL) {
            seperatedCommands[index] = currStr;
            isBG[index] = true;
            *brk = '\0';
            currStr = brk + 1;
            brk = strpbrkIgnoreQuotes(currStr, BG_COMMAND_SEPERATOR_TOKENS, QUOTES_TOKENS);
            index++;
        }
        if (*currStr != '\0') {
            seperatedCommands[index] = currStr;
            isBG[index] = false;
            index++;
        }
    }

    for (int i = 0; seperatedCommands[i] != NULL; i++) {
        handlePipedCommands(seperatedCommands[i], isBG[i]);
    }
}

// ========= main.h =========

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

// ========= main.c =========

int runShell() {
    signal(SIGTSTP, catch_SIGTSTP);
    // Caculate maximum possible commands
    const int MAX_POSSIBLE_COMMANDS = MAX_INPUT_LEN / 2 + 1;

    // Initialize GlobalVariables
    initGlobalVars();

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
        printf("cmd:%d|#args:%d@%s>", getCommandCount(), getArgumentCount(), currentDir);
        if (request_input_from_user(strInput, MAX_INPUT_LEN)) {
            continue;
        }

        printExited();
        if (strInput[0] == '\0') {
            counter++;
            continue;
        }
        else
            counter = 0;

        replaceVariablesWithValues(strInput);
        handleWholeInput(strInput);
    }

    freeGlobalVars();
    return 0;

MEMORY_ALLOC_ERR:
    fatalErrorExit("memory allocation error");
}

int main() {
    return runShell();
}