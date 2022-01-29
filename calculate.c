#include "smart_calc_v1.h"

/* алгоритм парсинга строки для ПН */
double transform_string_to_argument_PN(const char **operation,
                                       int *checkTypeOperator) {
  double result = 0;
  if (**operation == ' ') *operation += 1;
  if (**operation == '+' && *checkTypeOperator == FAILURE) {
    result = PLUS;
    *checkTypeOperator = SUCCESS;
  } else if (**operation == '-' && *checkTypeOperator == FAILURE) {
    result = MINUS;
    *checkTypeOperator = SUCCESS;
  } else if (**operation == '*') {
    result = MULT;
    *checkTypeOperator = SUCCESS;
  } else if (**operation == '/') {
    result = DIV;
    *checkTypeOperator = SUCCESS;
  } else if (**operation == '^') {
    result = POW;
    *checkTypeOperator = SUCCESS;
  } else if (**operation == '%') {
    result = MOD;
    *checkTypeOperator = SUCCESS;
  } else if (**operation == '+' && *checkTypeOperator == SUCCESS) {
    result = UPLUS;
  } else if (**operation == '-' && *checkTypeOperator == SUCCESS) {
    result = UMINUS;
  } else if (**operation == 'c' && strncmp(*operation, "cos", 3) == 0) {
    result = COS;
    *operation += 2;
  } else if (**operation == 's' && strncmp(*operation, "sin", 3) == 0) {
    result = SIN;
    *operation += 2;
  } else if (**operation == 't' && strncmp(*operation, "tan", 3) == 0) {
    result = TAN;
    *operation += 2;
  } else if (**operation == 'a') {
    if (strncmp(*operation, "atan", 4) == 0) result = ATAN;
    if (strncmp(*operation, "acos", 4) == 0) result = ACOS;
    if (strncmp(*operation, "asin", 4) == 0) result = ASIN;
    *operation += 3;
  } else if (**operation == 's' && strncmp(*operation, "sqrt", 4) == 0) {
    result = SQRT;
    *operation += 3;
  } else if (**operation == 'l' && strncmp(*operation, "ln", 2) == 0) {
    result = LN;
    *operation += 1;
  } else if (**operation == 'l' && strncmp(*operation, "log", 3) == 0) {
    result = LOG;
    *operation += 2;
  } else if (**operation == '(') {
    result = OPEN_BRACKET;
  } else if (**operation == ')') {
    result = CLOSE_BRACKET;
  } else if (**operation == 'x') {
    result = X;
    *checkTypeOperator = FAILURE;
  } else if (**operation == '.') {
    result = SIGN;
  } else {
    result = NUMBER;
    *checkTypeOperator = FAILURE;
  }
  if (result != NUMBER) {
    *operation += 1;
  }
  return result;
}

/* определение констант операций */
double check_constant_number(const char **givenStr) {
  double number = 0;
  if (strncmp(*givenStr, "pi", 2) == 0) {
    number = M_PI;
    *givenStr += 2;
  } else if (**givenStr == 'e') {
    number = M_E;
    *givenStr += 1;
  } else {
    char *ptrEnd = NULL;
    number = strtod(*givenStr, &ptrEnd);
    *givenStr = ptrEnd;
  }
  return number;
}

/* определение приоритета */
double get_prioritet(double number) {
  double prioritet = 0;
  if (number >= PLUS && number <= MINUS)
    prioritet = LOW;
  else if (number >= MULT && number <= MOD && number != POW)
    prioritet = MIDLE;
  else if (number == POW)
    prioritet = MIDLE_LOW;
  else if (number >= UPLUS && number <= UMINUS)
    prioritet = MIDLE_PLUS;
  else if (number >= COS && number <= LOG)
    prioritet = HIGH;
  return prioritet;
}

/* расчет алгоритма ПН */
double result_PN(const char *givenStr) {
  Stack *stack = NULL;
  stack = convert_to_polish_notashion(givenStr);
  double result = calculate_to_PN(&stack, UNDEFIND);
  clean_stack(&stack);
  return result;
}

/* перевод строки в ПН */
Stack *convert_to_polish_notashion(const char *givenStr) {
  Stack *stackOperation = NULL;
  Stack *stackPolishNotation = NULL;
  Stack *stackResult = NULL;
  algorithm_convert_to_PN(givenStr, &stackOperation, &stackPolishNotation);
  stackResult = reverse_stack(&stackPolishNotation);
  clean_stack(&stackOperation);
  clean_stack(&stackPolishNotation);
  return stackResult;
}

/* алгоритм перевода в ПН */
void algorithm_convert_to_PN(const char *givenStr, Stack **stackOperation,
                             Stack **stackPolishNotation) {
  int checkTypeOperator = SUCCESS;
  while (*givenStr != '\0') {
    double numberPN =
        transform_string_to_argument_PN(&givenStr, &checkTypeOperator);
    if (numberPN == NUMBER) {
      double number = check_constant_number(&givenStr);
      push_value(stackPolishNotation, number, NUMBER);
    } else if (numberPN == X) {
      push_value(stackPolishNotation, X, X);
    } else if (numberPN >= PLUS && numberPN <= LOG) {
      algorithm_add_operation_in_PN(stackOperation, stackPolishNotation,
                                    numberPN);
    } else if (numberPN == OPEN_BRACKET) {
      push_value(stackOperation, numberPN, OPERATION);
    } else if (numberPN == CLOSE_BRACKET) {
      double numberTemp = pop_value(stackOperation);
      while (numberTemp != OPEN_BRACKET) {
        push_value(stackPolishNotation, numberTemp, OPERATION);
        numberTemp = pop_value(stackOperation);
      }
      if (get_size_stack(*stackPolishNotation) == 0)
        push_value(stackPolishNotation, 0.0, NUMBER);
    } else if (numberPN == SIGN) {
      push_value(stackOperation, SIGN, SIGN);
    }
  }
  stack_transfer(*stackOperation, stackPolishNotation);
}

/* алгоритм добавление операторов в стек */
void algorithm_add_operation_in_PN(Stack **stackOperation,
                                   Stack **stackPolishNotation, double number) {
  double priorityNumber = get_prioritet(number);
  if (get_size_stack(*stackOperation) > 0) {
    double priority = get_prioritet(peek_value(*stackOperation));
    if (priority >= priorityNumber) {
      while (get_size_stack(*stackOperation) > 0) {
        double numberTemp = peek_value(*stackOperation);
        priority = get_prioritet(numberTemp);
        if (priority >= priorityNumber) {
          pop_value(stackOperation);
          push_value(stackPolishNotation, numberTemp, OPERATION);
        } else {
          break;
        }
      }
    }
  }
  push_value(stackOperation, number, OPERATION);
}

/* перенос данных из одного стека в другой */
void stack_transfer(Stack *stackGive, Stack **stackNeed) {
  double number = 0;
  while (stackGive) {
    number = peek_value(stackGive);
    if (stackGive->operationOrValue == NUMBER)
      push_value(stackNeed, number, NUMBER);
    else if (stackGive->operationOrValue == OPERATION)
      push_value(stackNeed, number, OPERATION);
    else if (stackGive->operationOrValue == X)
      push_value(stackNeed, number, X);
    else if (stackGive->operationOrValue == SIGN)
      push_value(stackNeed, number, SIGN);
    stackGive = stackGive->next;
  }
}

Stack *reverse_stack(Stack **stackPolishNotation) {
  Stack *stackReverse = NULL;
  int sizeStack = get_size_stack(*stackPolishNotation);
  while (get_size_stack(*stackPolishNotation) > 0) {
    int element = peek_element(*stackPolishNotation);
    double number = pop_value(stackPolishNotation);
    push_value(&stackReverse, number, element);
  }
  return stackReverse;
}

/* расчет выражения из ПН */
double calculate_to_PN(Stack **stackPolishNotation, double valueX) {
  int check = SUCCESS;
  double result = 0;
  Stack *stackCalculate = NULL;
  Stack *stackTemp = NULL;
  stack_transfer(*stackPolishNotation, &stackTemp);
  stackTemp = reverse_stack(&stackTemp);
  replace_value_x_number(stackTemp, valueX);
  while (get_size_stack(stackTemp) > 0) {
    int checkOperatorOrValue = peek_element(stackTemp);
    double number = pop_value(&stackTemp);
    if (checkOperatorOrValue == NUMBER)
      push_value(&stackCalculate, number, NUMBER);
    else if (number >= PLUS && number <= MOD)
      work_calculate_binary(&stackCalculate, &number, &check);
    else if (number >= UPLUS && number <= LOG)
      work_calculate_unary(&stackCalculate, &number, &check);
    else if (checkOperatorOrValue == SIGN)
      check = FAILURE;
    if (check == FAILURE) {
      push_value(&stackCalculate, 0, NUMBER);
      break;
    }
  }
  result = pop_value(&stackCalculate);
  clean_stack(&stackCalculate);
  clean_stack(&stackTemp);
  return result;
}

/* расчет бинарные арифметические операции */
void work_calculate_binary(Stack **stackCalculate, double *number, int *check) {
  if (get_size_stack(*stackCalculate) > 1) {
    double result = 0;
    double numberTwo = pop_value(stackCalculate);
    double numberOne = pop_value(stackCalculate);
    if (*number == PLUS)
      result = numberOne + numberTwo;
    else if (*number == MINUS)
      result = numberOne - numberTwo;
    else if (*number == MULT)
      result = numberOne * numberTwo;
    else if (*number == DIV)
      result = numberOne / numberTwo;
    else if (*number == POW)
      result = pow(numberOne, numberTwo);
    else if (*number == MOD)
      result = fmod(numberOne, numberTwo);
    push_value(stackCalculate, result, NUMBER);
  } else {
    *check = FAILURE;
  }
}

/* расчет унарных арифметические операции */
void work_calculate_unary(Stack **stackCalculate, double *number, int *check) {
  if (get_size_stack(*stackCalculate) > 0) {
    double result = 0;
    double numberStack = pop_value(stackCalculate);
    if (*number == UPLUS)
      result = numberStack;
    else if (*number == UMINUS)
      result = -1 * numberStack;
    else if (*number == COS)
      result = cos(numberStack);
    else if (*number == SIN)
      result = sin(numberStack);
    else if (*number == TAN)
      calculate_tan(&result, numberStack);
    else if (*number == ATAN)
      result = atan(numberStack);
    else if (*number == ACOS)
      result = acos(numberStack);
    else if (*number == ASIN)
      result = asin(numberStack);
    else if (*number == SQRT)
      result = sqrt(numberStack);
    else if (*number == LN)
      result = log(numberStack);
    else if (*number == LOG)
      result = log10(numberStack);
    push_value(stackCalculate, result, NUMBER);
  } else {
    *check = FAILURE;
  }
}

/* расчет тангенса */
void calculate_tan(double *result, double numberStack) {
  if (tan(numberStack) >= 0x7ff00000 || tan(numberStack) < -0x7ff00000)
    *result = NAN;
  else
    *result = tan(numberStack);
}

/* добавить элемент в стек */
void push_value(Stack **head, double value, int operationOrValue) {
  Stack *stackTmp = malloc(sizeof(Stack));
  if (stackTmp == NULL) exit(0);
  stackTmp->next = *head;
  stackTmp->value = value;
  stackTmp->operationOrValue = operationOrValue;
  *head = stackTmp;
}

/* достать элемент из стека */
double pop_value(Stack **head) {
  Stack *out;
  double value;
  if (*head == NULL) exit(0);
  out = *head;
  *head = (*head)->next;
  value = out->value;
  free(out);
  return value;
}

/* посмотреть первый элемент в стеке */
double peek_value(const Stack *head) {
  if (head == NULL) exit(0);
  return head->value;
}

/* посмотреть тип элемента в стеке */
int peek_element(const Stack *head) {
  if (head == NULL) exit(0);
  return head->operationOrValue;
}

/* узнать размер стека */
size_t get_size_stack(const Stack *head) {
  size_t size = 0;
  while (head) {
    size++;
    head = head->next;
  }
  return size;
}

void clean_stack(Stack **head) {
  while (get_size_stack(*head) > 0) {
    pop_value(head);
  }
}

/* замена X на значение в стеке ПН */
void replace_value_x_number(Stack *stackPolishNotation, double valueX) {
  while (stackPolishNotation) {
    if (stackPolishNotation->operationOrValue == X) {
      stackPolishNotation->value = valueX;
      stackPolishNotation->operationOrValue = NUMBER;
    }
    stackPolishNotation = stackPolishNotation->next;
  }
}

/* проверка x в стеке ПН */
int check_value_x_number(const Stack *stackPolishNotation) {
  int result = FAILURE;
  while (stackPolishNotation) {
    if (stackPolishNotation->operationOrValue == X) {
      result = SUCCESS;
      break;
    }
    stackPolishNotation = stackPolishNotation->next;
  }
  return result;
}
