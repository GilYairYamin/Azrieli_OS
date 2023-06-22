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
#include <pthread.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define SEMAPHORE_NAME "/polinomials_semaphore"
#define SHARED_KEY "/tmp"

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
 * @brief Perform a polinomial operation on 2 polinomials and return the result.
 * where op represents the operation:
 * '+' : addition
 * '-' : subtraction
 * '*' : multiplication
 *
 * @param leftOperand - the left operand of the operation.
 * @param rightOperand - the right operand of the operation.
 * @param op - a character representing the operation as described above.
 * @return Polinomial* - a polinomial representing the result of the operation.
 */
Polinomial* polinomials_operation(Polinomial* leftOperand, Polinomial* rightOperand, char op);

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
 * @brief Add or subtract two numbers based on the operation op.
 * '+' : addition.
 * '-' : subtraction.
 *
 * @param leftOperand - the left operand.
 * @param rightOperand - the right operand.
 * @param op - addition or subtraction.
 * @return int - the result.
 */
int add_sub_element(int leftOperand, int rightOperand, char op) {
    switch (op) {
    case '+':
        return leftOperand + rightOperand;
    case '-':
        return leftOperand - rightOperand;
    }
    return 0;
}

pthread_mutex_t mtx;
Polinomial* global_left_operand = NULL, * global_right_operand = NULL, * global_result = NULL;
char global_op = '\0';

void* calculate_thread(void* index) {
    int i = *((int*)index);
    Polinomial* a = global_left_operand;
    Polinomial* b = global_right_operand;
    Polinomial* res = global_result;
    char op = global_op;

    pthread_mutex_lock(&mtx);

    int left = (i <= a->degree) ? a->coefficients[a->degree - i] : 0;
    int right = (i <= b->degree) ? b->coefficients[b->degree - i] : 0;
    res->coefficients[res->degree - i] = add_sub_element(left, right, op);

    pthread_mutex_unlock(&mtx);
    return 0;
}

Polinomial* add_sub_polinomials(Polinomial* a, Polinomial* b, char op) {
    if (op != '+' && op != '-')
        return NULL;

    int degree = (a->degree > b->degree) ? a->degree : b->degree;
    Polinomial* res = create_polinomial(degree, NULL);
    if (res == NULL)
        return NULL;

    pthread_mutex_init(&mtx, NULL);
    global_left_operand = a;
    global_right_operand = b;
    global_result = res;
    global_op = op;

    int thread_amount = degree + 1;
    int status = 0;
    int* thread_nums = calloc(sizeof(int), thread_amount);
    pthread_t* threads = calloc(sizeof(pthread_t), thread_amount);

    if (thread_nums == NULL || threads == NULL) {
        fprintf(stderr, "memory allocation error");
        free(thread_nums);
        free(threads);
        free(res);
        return NULL;
    }

    int err = 0;
    for (int i = 0; i <= degree; i++) {
        thread_nums[i] = i;

        status = pthread_create(&(threads[i]), NULL, calculate_thread, (void*)(&thread_nums[i]));
        if (status != 0) {
            perror("pthread create failed");
            err = 1;
            goto EXIT;
        }
    }


EXIT:
    pthread_mutex_destroy(&mtx);
    for (int i = 0; i < thread_amount; i++) {
        if (threads[i] != 0)
            pthread_join(threads[i], NULL);
    }

    global_left_operand = NULL;
    global_right_operand = NULL;
    global_result = NULL;
    global_op = '\0';

    free(thread_nums);
    free(threads);
    if (err) {
        free_polinomial(res);
        return NULL;
    }
    return res;
}

/**
 * @brief multiply two polinomials.
 *
 * @param leftOperand - the left operand.
 * @param rightOperand - the right operand.
 * @return Polinomial* - the result.
 */
Polinomial* mul_polinomials(Polinomial* leftOperand, Polinomial* rightOperand) {
    int degree = leftOperand->degree + rightOperand->degree;
    Polinomial* res = create_polinomial(degree, NULL);

    int aNum = 0, bNum = 0, resNum;
    for (int i = 0; i <= leftOperand->degree; i++) {
        for (int j = 0; j <= rightOperand->degree; j++) {
            aNum = leftOperand->coefficients[leftOperand->degree - i];
            bNum = rightOperand->coefficients[rightOperand->degree - j];
            resNum = aNum * bNum;
            res->coefficients[degree - i - j] += resNum;
        }
    }
    return res;
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

Polinomial* polinomials_operation(Polinomial* leftOperand, Polinomial* rightOperand, char op) {
    switch (op) {
    case '+':
    case '-':
        return add_sub_polinomials(leftOperand, rightOperand, op);
    case '*':
        return mul_polinomials(leftOperand, rightOperand);
    }
    return NULL;
}

void free_polinomial(Polinomial* p) {
    if (p == NULL)
        return;
    free(p);
}

// ======= END ========
// ======= shared_content.h ========

#define MAX_COEFFICIENT 128
#define MAX_SHARED_OPERATIONS 10
#define SHARED_CONTENT_SIZE 1280

#define shared_arr_t short

/**
 * @brief defines a local operation.
 *
 */
typedef struct {
    char op;
    Polinomial* p1;
    Polinomial* p2;
} LocalOperation;

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
 * @brief copt shared content operations to this process's internal memory.
 *
 * @param sc - shared content.
 * @param input - local input.
 * @return int 0 if successful, 1 otherwise.
 */
int copy_shared_content(SharedContent* sc, LocalOperation input[]);

/**
 * @brief clears the shared memory space from all operations.
 *
 * @param sc - share memory space.
 */
void clear_shared_content(SharedContent* sc);

/**
 * @brief decode polinomial from polinomials in shared content to regular polinomial object.
 *
 * @param arr
 * @return Polinomial*
 */
Polinomial* decode_polinomial(shared_arr_t arr[]);



// ======= END ========
// ======= shared_content.c ========

Polinomial* decode_polinomial(shared_arr_t arr[]) {
    int intArr[MAX_COEFFICIENT];
    intArr[0] = arr[0];
    for (int i = 0; i <= intArr[0]; i++) {
        intArr[i + 1] = arr[i + 1];
    }
    return create_polinomial(arr[0], intArr + 1);
}

void clear_shared_content(SharedContent* sc) {
    sc->end = false;
    for (int i = 0; i < MAX_SHARED_OPERATIONS; i++) {
        sc->operations[i].op = '\0';
        sc->operations[i].leftPoliOffset = 0;
        sc->operations[i].rightPoliOffest = 0;
    }
}

int copy_shared_content(SharedContent* sc, LocalOperation input[]) {
    for (int i = 0; i < MAX_SHARED_OPERATIONS && sc->operations[i].op != '\0'; i++) {
        input[i].op = sc->operations[i].op;
        input[i].p1 = decode_polinomial(&(sc->polinomials[sc->operations[i].leftPoliOffset]));
        input[i].p2 = decode_polinomial(&(sc->polinomials[sc->operations[i].rightPoliOffest]));
    }
    return 0;
}

// ======= END =======
// ======= ex3q3a.h ========

/**
 * @brief wait for input from shared content.
 * When input detected, copy it to the process's internal storage and clear the shared content.
 * If shared content signal the main function to end by making input[0].op = '\0'
 *
 * @param sc
 * @param semaphore
 * @param input
 * @return int
 */
int wait_for_input(SharedContent* sc, sem_t* semaphore, LocalOperation input[]);

/**
 * @brief clear the entire input object from any currently held operations.
 *
 * @param operations
 */
void clear_input(LocalOperation operations[]);

/**
 * @brief take the input, calculate the operations, and print the result.
 *
 * @param operations
 * @return int
 */
int handle_input(LocalOperation operations[]);

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
void close_shared_enviornment(sem_t* semaphore, void* sm);



// ======= END =======
// ======= ex3q3b.c ========

void clear_input(LocalOperation operations[]) {
    for (int i = 0; i < MAX_SHARED_OPERATIONS; i++) {
        if (operations[i].op == '\0')
            continue;

        free_polinomial(operations[i].p1);
        free_polinomial(operations[i].p2);

        operations[i].p1 = NULL;
        operations[i].p2 = NULL;
        operations[i].op = '\0';
    }
}

int wait_for_input(SharedContent* sc, sem_t* semaphore, LocalOperation input[]) {
    bool waiting = true;
    do {
        usleep(rand() % 50 + 50);
        sem_wait(semaphore);
        if (sc->operations[0].op != '\0') {
            copy_shared_content(sc, input);
            clear_shared_content(sc);
            waiting = false;
        }
        if (sc->end == true) {
            waiting = false;
            input[0].op = '\0';
        }
        sem_post(semaphore);
    } while (waiting);
    return 0;
}

int setup_shared_enviornment(sem_t** semaphore, int* shm_id, void** shared_memory) {
    *semaphore = sem_open(SEMAPHORE_NAME, 0);
    if (*semaphore == SEM_FAILED) {
        perror("sem_open");
        goto ERROR_EXIT;
    }

    key_t key = ftok(SHARED_KEY, 'y');
    if (key == -1) {
        perror("ftok failed");
        goto ERROR_EXIT;
    }

    *shm_id = shmget(key, 0, 0600);
    if (*shm_id == -1) {
        perror("shmget failed");
        goto ERROR_EXIT;
    }

    *shared_memory = shmat(*shm_id, NULL, 0);
    if (*shared_memory == (void*)-1) {
        perror("share memory");
        goto ERROR_EXIT;
    }
    return 0;

ERROR_EXIT:
    return 1;
}

void close_shared_enviornment(sem_t* semaphore, void* shared_memory) {
    if (shared_memory != (void*)-1 && shmdt(shared_memory) == -1)
        perror("shmdt");

    if (semaphore != (sem_t*)-1) {
        sem_post(semaphore);
        if (sem_close(semaphore) == -1)
            perror("sem_close");
    }
}

int handle_input(LocalOperation operations[]) {
    Polinomial* res;
    for (int i = 0; i < MAX_SHARED_OPERATIONS; i++) {
        if (operations[i].op == '\0')
            continue;

        res = polinomials_operation(operations[i].p1, operations[i].p2, operations[i].op);
        print_polinomial(res);
        free_polinomial(res);
    }
    return 0;
}

int main() {
    srand(time(NULL));
    sem_t* semaphore = SEM_FAILED;
    int shm_id = -1;
    void* shared_memory = (void*)-1;

    if (setup_shared_enviornment(&semaphore, &shm_id, &shared_memory)) {
        goto ERROR_EXIT;
    }

    SharedContent* sc = (SharedContent*)shared_memory;
    LocalOperation operations[MAX_SHARED_OPERATIONS] = { 0 };

    while (1) {
        if (wait_for_input(sc, semaphore, operations) != 0) {
            fprintf(stderr, "no provider was found\n");
            goto ERROR_EXIT;
        }
        if (operations[0].op == '\0') {
            break;
        }
        handle_input(operations);
        clear_input(operations);
    }

    clear_input(operations);
    close_shared_enviornment(semaphore, shared_memory);
    exit(EXIT_SUCCESS);

ERROR_EXIT:
    close_shared_enviornment(semaphore, shared_memory);
    exit(EXIT_FAILURE);
}