#ifndef SRC_SMART_CALC_V1_H_
#define SRC_SMART_CALC_V1_H_

#define SUCCESS 1
#define FAILURE 0
#define SIGN -11

#define EPS 1e-7

#define CONSTANT_OPERATION 3
#define NUMBER 0
#define OPERATION 1
#define UOPERATION 2
#define UNDEFIND -10
#define X -3

#define PLUS 1
#define MINUS 2
#define MULT 3
#define DIV 4
#define POW 5
#define MOD 6
#define UPLUS 7
#define UMINUS 8
#define COS 9
#define SIN 10
#define TAN 11
#define ATAN 12
#define ACOS 13
#define ASIN 14
#define SQRT 15
#define LN 16
#define LOG 17
#define OPEN_BRACKET -1
#define CLOSE_BRACKET -2

#define LOW 1
#define MIDLE 2
#define MIDLE_LOW 3
#define MIDLE_PLUS 4
#define HIGH 5

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Stack {
  double value;
  int operationOrValue;
  struct Stack *next;
} Stack;

typedef struct Credit {
  double overpayment;
  double minMonthlyPayment;
  double maxMonthlyPayment;
  double monthlyPayment;
  double interestCharges;
  int check;
} Credit;

typedef struct CreditSting {
  char *sumCredit;
  char *percentCredit;
  char *termCredit;
  char *typeCredit;
  char *monthOrYear;
} CreditSting;

/* функции для работы со стеком double */
void push_value(Stack **head, double value, int operationOrValue);
double pop_value(Stack **head);
double peek_value(const Stack *head);
int peek_element(const Stack *head);
void print_stack(const Stack *head);
size_t get_size_stack(const Stack *head);
void clean_stack(Stack **head);

/* функции работы алгоритма ПН */
double transform_string_to_argument_PN(const char **operation,
                                       int *checkTypeOperator);
double get_prioritet(double number);
Stack *convert_to_polish_notashion(const char *givenStr);
void algorithm_convert_to_PN(const char *givenStr, Stack **stackOperation,
                             Stack **stackPolishNotation);
void algorithm_add_operation_in_PN(Stack **stackOperation,
                                   Stack **stackPolishNotation, double number);
void stack_transfer(Stack *stackOperation, Stack **stackPolishNotation);
Stack *reverse_stack(Stack **stackPolishNotation);
double calculate_to_PN(Stack **stackPolishNotation, double valueX);
void work_calculate_binary(Stack **stackCalculate, double *number, int *check);
void work_calculate_unary(Stack **stackCalculate, double *number, int *check);
double check_constant_number(const char **givenStr);
int check_value_x_number(const Stack *stackPolishNotation);
void replace_value_x_number(Stack *stackPolishNotation, double valueX);
void calculate_tan(double *result, double numberStack);
int check_stack_value_operation_to_plot(const Stack *stackPolishNotation);
double result_PN(const char *givenStr);

/* функции работы с кредитами */
Credit credit_annuity_work(double sumCredit, double percentCredit,
                           double termCredit);
Credit credit_differentiated_work(double sumCredit, double percentCredit,
                                  double termCredit);
void init_credit(Credit *credit);
int covert_sting_to_double_credit(CreditSting *creditString, double *sumCredit,
                                  double *percentCredit, double *termCredit);
Credit credit_work(CreditSting *creditString);
int check_credit(CreditSting *creditString, double *sumCredit,
                 double *percentCredit, double *termCredit);

#endif  // SRC_SMART_CALC_V1_H_
