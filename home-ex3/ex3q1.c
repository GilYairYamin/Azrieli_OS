// ======== general_includes.h ========
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

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

/**
 * @brief add or subtract two polinomials based on op.
 * '+' : addition.
 * '-' : subtraction.
 *
 * @param leftOperand - left operand.
 * @param rightOperand - right operand.
 * @param op - the operatrion.
 * @return Polinomial* - the result.
 */
Polinomial* add_sub_polinomials(Polinomial* leftOperand, Polinomial* rightOperand, char op) {
    int degree = (leftOperand->degree > rightOperand->degree) ? leftOperand->degree : rightOperand->degree;
    Polinomial* res = create_polinomial(degree, NULL);

    int aNum = 0, bNum = 0, resNum;
    for (int i = 0; i <= degree; i++) {
        aNum = (i <= leftOperand->degree) ? leftOperand->coefficients[leftOperand->degree - i] : 0;
        bNum = (i <= rightOperand->degree) ? rightOperand->coefficients[rightOperand->degree - i] : 0;
        resNum = add_sub_element(aNum, bNum, op);
        res->coefficients[degree - i] = resNum;
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
// ======= ex3q1.h ========

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
 * @brief the function recieves an input, translates it to polinomials and an operation
 * and prints the result of the operation.
 *
 * @param str - the input.
 * @return int - 0 if the operation was successful, 1 otherwise.
 */
int handle_input(char str[]);

// ======= END ========
// ======= ex3q1.c =======

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

int handle_input(char str[]) {
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
    res = polinomials_operation(p1, p2, op);
    print_polinomial(res);
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

int main() {
    char str[MAX_INPUT_LENGTH];
    str[0] = '\0';

    while (1) {
        request_input_from_user(str, MAX_INPUT_LENGTH);
        if (strcmp(str, "END") == 0)
            break;
        handle_input(str);
    }
}