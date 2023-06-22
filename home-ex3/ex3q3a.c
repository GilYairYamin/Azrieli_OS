// ======== general_includes.h ========
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define SEMAPHORE_NAME "/polinomials_semaphore"
#define SHARED_KEY "/tmp"

#define MAX_INPUT_LENGTH 128

// ======== END ========
// ======== polinomial.h ========

/**
 * @brief Defines a polinomial with a degree and array of coefficients.
 *
 */
typedef struct {
    unsigned int degree;
    int coefficients[];
} Polinomial;

/**
 * @brief Create a polinomial object.
 *
 * @param degree - the degree of the polinomial.
 * @param coefficients - the coefficients of the polinomial, can send NULL to represent a list of 0s.
 * @return Polinomial* - the polinomial object.
 */
Polinomial* create_polinomial(int degree, int* coefficients);

/**
 * @brief Parse string to a polinomial object.
 *
 * @param str - A string representing a polinomial of the following format:
 * <degree>:<num>,<num>,...,<num>
 * where the amount of "nums" is the degree + 1, and each num represents the
 * coefficient of the largest power in the polinomial to the smallest from left to right.
 *
 * @return Polinomial* - a polinomial object represented by the string str.
 */
Polinomial* parse_str_to_plinomial(char str[]);

/**
 * @brief Print a polinomial.
 *
 * @param p - polinomial to print.
 */
void print_polinomial(Polinomial* p);

/**
 * @brief free a polinomial object.
 *
 * @param p - a polinomial object to free.
 */
void free_polinomial(Polinomial* p);

// ======== END ========
// ======== polinomial.c ========


/**
 * @brief check whether a string represents a whole number.
 *
 * @param str a string to check.
 * @return true - if the string represents a whole number.
 * @return false - if the string doesn't represent a whole number.
 */
bool isnumber(char str[]) {
    int index = (str[0] == '-') ? 1 : 0;

    if (str[index] == '\0')
        return false;

    while (str[index] != '\0') {
        if (!isdigit(str[index++]))
            return false;
    }

    return true;
}

/**
 * @brief initialize a polinomial using a degree and array of coefficients.
 *
 * @param p - the polinomial to initialize.
 * @param degree - the degree of the polinomial.
 * @param coefficients - and array of coefficients.
 * @return int - returns 0 if succesfully initialized p, 1 otherwise.
 */
int init_polinomial(Polinomial* p, int degree, int coefficients[]) {
    p->degree = degree;
    if (coefficients == NULL)
        return 0;
    for (int i = 0; i <= degree; i++) {
        p->coefficients[i] = coefficients[i];
    }
    return 0;
}

/**
 * @brief print a single element of a polinomial with its coefficient and power.
 *
 * @param coefficient - the coefficient.
 * @param power - the power.
 */
void print_coefficient(int coefficient, int power) {
    if (power == 0) {
        printf("%d", coefficient);
        return;
    }
    if (coefficient != 1)
        printf("%d", coefficient);
    printf("X");
    if (power != 1)
        printf("^%d", power);
}

Polinomial* create_polinomial(int degree, int coefficients[]) {
    int len = degree + 1;
    Polinomial* res = calloc(sizeof(Polinomial) + (sizeof(int) * len), 1);
    if (res == NULL) {
        return NULL;
    }
    if (init_polinomial(res, degree, coefficients)) {
        free(res);
        return NULL;
    }
    return res;
}

void print_polinomial(Polinomial* p) {
    int i = 0;
    char sign;
    for (; i <= p->degree; i++) {
        if (i < p->degree && p->coefficients[i] == 0)
            continue;
        if (p->coefficients[i] < 0)
            printf("-");
        print_coefficient(abs(p->coefficients[i]), p->degree - i);
        break;
    }

    for (i++; i <= p->degree; i++) {
        if (p->coefficients[i] == 0)
            continue;
        sign = (p->coefficients[i] >= 0) ? '+' : '-';
        printf(" %c ", sign);
        print_coefficient(abs(p->coefficients[i]), p->degree - i);
    }
    printf("\n");
}


Polinomial* parse_str_to_plinomial(char str[]) {
    char* curr = strchr(str, ':');
    if (curr == NULL) {
        return NULL;
    }

    if (strchr(curr + 1, ':') != NULL) {
        return NULL;
    }

    *curr = '\0';
    if (isnumber(str) == 0) {
        return NULL;
    }

    int degree = atoi(str);
    int coefficients[MAX_INPUT_LENGTH] = { 0 };

    int index = 0;
    curr = curr + 1;
    char* next = strchr(curr, ',');
    while (next != NULL) {
        *next = '\0';

        if (isnumber(curr) == 0) {
            return NULL;
        }

        coefficients[index++] = atoi(curr);
        curr = next + 1;
        if (*curr == '\0') {
            return NULL;
        }
        next = strchr(curr, ',');
    }

    if (isnumber(curr) == 0) {
        return NULL;
    }

    coefficients[index++] = atoi(curr);
    if (index > degree + 1) {
        fprintf(stderr, "too many coefficients\n");
        return NULL;
    }
    else if (index < degree + 1) {
        fprintf(stderr, "missing coefficients\n");
        return NULL;
    }

    Polinomial* res = create_polinomial(degree, coefficients);
    return res;
}

void free_polinomial(Polinomial* p) {
    if (p == NULL)
        return;
    free(p);
}

// ======= END ========
// ======= shared_content.h ========

#define MAX_SHARED_OPERATIONS 10
#define SHARED_CONTENT_SIZE 1280

#define shared_arr_t short

/**
 * @brief defines a representation of an operation within the shared memory space.
 * op - represents the operation, addition, subtraction or multiplication.
 * leftPoliOffset - where in the polinomials array the left operand polinomial begins.
 * rightPoliOffest - where in the polinomials array the right operand polinomial begins.
 *
 */
typedef struct {
    char op;
    unsigned short leftPoliOffset;
    unsigned short rightPoliOffest;
} SharedOperation;

/**
 * @brief defines the share memory spaces where
 * end - represents whether the provider finished its operation.
 * operations - represents all the operations saved in the shared memory.
 *
 * polinomials - an array that saves all the polinomials in the shared memory,
 * where each polinomial begins with its degree, and following its coefficients.
 * the beginning of each polinomial is saved in the operations poliOffsets.
 *
 */
typedef struct {
    bool end;
    SharedOperation operations[MAX_SHARED_OPERATIONS];
    shared_arr_t polinomials[];
} SharedContent;

/**
 * @brief returns the last offset possible within the shared memory space.
 *
 * @return int the last possible offset within the shared memory space.
 */
int max_offset();

/**
 * @brief clears the shared memory space from all operations.
 *
 * @param sc - share memory space.
 */
void clear_shared_content(SharedContent* sc);

/**
 * @brief encode a polinomial inside an array of numbers.
 *
 * @param p the polinomial.
 * @param arr the array (address where the polinomial should start).
 * @return int 0 if the operation was successful, 1 otherwise.
 */
int encode_polinomial(Polinomial* p, shared_arr_t arr[]);

// ======= END ========
// ======= shared_content.c ========

int max_offset() {
    return (SHARED_CONTENT_SIZE - sizeof(SharedContent)) / sizeof(shared_arr_t);
}

void clear_shared_content(SharedContent* sc) {
    sc->end = false;
    for (int i = 0; i < MAX_SHARED_OPERATIONS; i++) {
        sc->operations[i].op = '\0';
        sc->operations[i].leftPoliOffset = 0;
        sc->operations[i].rightPoliOffest = 0;
    }
}

int encode_polinomial(Polinomial* p, shared_arr_t arr[]) {
    arr[0] = p->degree;
    for (int i = 0; i <= arr[0]; i++) {
        arr[i + 1] = p->coefficients[i];
    }
    return 0;
}

int encode_operation(SharedContent* sc, Polinomial* p1, Polinomial* p2, char op) {
    int index = 0;
    for (index = 0; index < MAX_SHARED_OPERATIONS && sc->operations[index].op != '\0'; index++);
    if (index >= MAX_SHARED_OPERATIONS) {
        fprintf(stderr, "too many operations in shared memory\n");
        return 1;
    }

    int nextOffset = 0;
    if (index > 0) {
        int lastOffset = sc->operations[index - 1].rightPoliOffest;
        int lastLength = sc->polinomials[lastOffset] + 2;
        nextOffset = lastOffset + lastLength;
    }

    int p1Len = p1->degree + 2;
    int p2Len = p2->degree + 2;

    if (nextOffset + p1Len + p2Len >= max_offset()) {
        fprintf(stderr, "not enough storage in shared memory\n");
        return 1;
    }

    int p1Offset = nextOffset;
    int p2Offset = p1Offset + p1Len;

    sc->operations[index].op = op;
    sc->operations[index].leftPoliOffset = p1Offset;
    sc->operations[index].rightPoliOffest = p2Offset;

    encode_polinomial(p1, &(sc->polinomials[p1Offset]));
    encode_polinomial(p2, &(sc->polinomials[p2Offset]));
    return 0;
}

// ======== END ========
// ======== ex3q3a.h ========

/**
 * @brief split a string to multiple string each representing a polinomial or an operation.
 * (<polinomial>)<operation>(<polinomial>)
 * where polinomial is defined in parse_str_to_polinomial.
 * and operation may be ADD, SUB or MUL.
 * "ADD" : '+' (addition)
 * "SUB" : '-' (subtraction)
 * "MUL" : '*' (multiplication)
 *
 * @param str a string representing multiple polinomials and operations between them.
 * @param splits the resulting split array with pointers to all elements
 * @return int 0 - if the operation was successful, 1 otherwise.
 */
int split_input_to_operation_and_polinomials(char str[], char* splits[]);

/**
 * @brief takes a string representing an operation, and translates it to one character.
 * "ADD" : '+' (addition)
 * "SUB" : '-' (subtraction)
 * "MUL" : '*' (multiplication)
 *
 * @param str - the string representing an operation.
 * @return char -  a character representing the same operation. '\0' if the string doesn't represent a known operation.
 */
char parse_operation(char str[]);

/**
 * @brief @brief the function recieves an input, translates it to polinomials and an operation
 * and saves the operation into the shared memory space.
 *
 * @param str - the input
 * @param semaphore - the semaphore for synchronous shared memory access.
 * @param sc - shared content object.
 * @return int - 0 if successful, 1 otherwise.
 */
int handle_input(char str[], sem_t* semaphore, SharedContent* sc);

/**
 * @brief Set the up shared enviornment, inclusing shared memory and semaphore.
 *
 * @param semaphore - pointer to semaphore.
 * @param shm_id - pointer to shm_id.
 * @param sm - pointer to shared memory.
 * @return int 0 if successful, 1 otherwise.
 */
int setup_shared_enviornment(sem_t** semaphore, int* shm_id, void** sm);

/**
 * @brief close the shared enviornement.
 *
 * @param semaphore - semaphore to close.
 * @param shm_id - shm_id to close.
 * @param sm - shared memory to close.
 */
void close_shared_enviornment(sem_t* semaphore, int shm_id, void* sm);

/**
 * @brief Request input from the user, will be saved in the buffer buff.
 *
 * @param buff buffer where input will be saved.
 * @param n the maximum length of the buffer.
 * @return int 0 if the request was successful, error code otherwise.
 */
int request_input_from_user(char buff[], int n);

// ======== END ========
// ======== ex3q3a.c ========

int split_input_to_operation_and_polinomials(char str[], char* splits[]) {
    char* curr = str;
    char* next;
    int i = 0;
    while (curr != NULL) {
        if (*curr != '(') {
            fprintf(stderr, "polinomial err ");
            return 1;
        }

        *curr = '\0';
        curr = curr + 1;
        splits[i++] = curr;
        next = strchr(curr, ')');
        if (next == NULL) {
            fprintf(stderr, "didn't find expected ) ");
            return 1;
        }

        *next = '\0';
        next = next + 1;
        if (*next == '\0') {
            break;
        }

        splits[i++] = next;
        curr = strchr(next, '(');
        if (curr == NULL) {
            fprintf(stderr, "didn't find expected (\n");
            return 1;
        }
    }
    splits[i] = NULL;
    return 0;
}

char parse_operation(char str[]) {
    if (strcmp(str, "ADD") == 0)
        return '+';

    if (strcmp(str, "SUB") == 0)
        return '-';

    if (strcmp(str, "MUL") == 0)
        return '*';

    return '\0';
}

int setup_shared_enviornment(sem_t** semaphore, int* shm_id, void** sm) {
    *semaphore = sem_open(SEMAPHORE_NAME, O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (*semaphore == SEM_FAILED) {
        perror("sem_open");
        goto ERROR_EXIT;
    }

    key_t key = ftok(SHARED_KEY, 'y');
    if (key == -1) {
        perror("ftok failed");
        goto ERROR_EXIT;
    }

    *shm_id = shmget(key, SHARED_CONTENT_SIZE, IPC_CREAT | IPC_EXCL | 0600);
    if (*shm_id == -1) {
        perror("shmget failed");
        goto ERROR_EXIT;
    }

    *sm = shmat(*shm_id, NULL, 0);
    if (*sm == (void*)-1) {
        perror("share memory");
        goto ERROR_EXIT;
    }
    return 0;

ERROR_EXIT:
    return 1;
}

void close_shared_enviornment(sem_t* semaphore, int shm_id, void* sm) {
    if (sm != (void*)-1 && shmdt(sm) == -1)
        perror("shmdt");

    if (shm_id != -1 && shmctl(shm_id, IPC_RMID, 0))
        perror("shmctl");

    if (semaphore != (sem_t*)-1) {
        sem_post(semaphore);
        if (sem_close(semaphore) == -1)
            perror("sem_close");
    }

    if (sem_unlink(SEMAPHORE_NAME) == -1)
        perror("sem_unlink");
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
        return 1;
    }

    buff[strlen(buff) - 1] = '\0';
    return 0;
}

int handle_input(char str[], sem_t* semaphore, SharedContent* sc) {
    int err = 0;
    Polinomial* p1 = NULL, * p2 = NULL, * res = NULL;
    char* splits[MAX_INPUT_LENGTH];

    if (split_input_to_operation_and_polinomials(str, splits)) {
        goto INVALID_INPUT;
    }

    if (splits[0] == NULL || splits[1] == NULL || splits[2] == NULL || splits[3] != NULL) {
        goto INVALID_INPUT;
    }

    p1 = parse_str_to_plinomial(splits[0]);
    p2 = parse_str_to_plinomial(splits[2]);

    char op = parse_operation(splits[1]);
    if (p1 == NULL || p2 == NULL || op == '\0') {
        goto INVALID_INPUT;
    }
    sem_wait(semaphore);

    if (encode_operation(sc, p1, p2, op)) {
        err = 1;
    }

    sem_post(semaphore);
    goto END;

INVALID_INPUT:
    err = 1;
    fprintf(stderr, "invalid input\n");

END:
    free_polinomial(p1);
    free_polinomial(p2);
    free_polinomial(res);
    return err;
}

int main() {
    srand(time(NULL));
    sem_t* semaphore = (sem_t*)-1;
    int shm_id = -1;
    void* shared_memory = (void*)-1;

    if (setup_shared_enviornment(&semaphore, &shm_id, &shared_memory)) {
        goto ERROR_EXIT;
    }

    SharedContent* sc = (SharedContent*)shared_memory;
    clear_shared_content(sc);
    char input[MAX_INPUT_LENGTH];
    input[0] = '\0';

    while (1) {
        request_input_from_user(input, MAX_INPUT_LENGTH);
        if (strcmp(input, "END") == 0) {
            sc->end = true;
            break;
        }
        handle_input(input, semaphore, sc);
    }

    close_shared_enviornment(semaphore, shm_id, shared_memory);
    exit(EXIT_SUCCESS);

ERROR_EXIT:
    close_shared_enviornment(semaphore, shm_id, shared_memory);
    exit(EXIT_FAILURE);
}
