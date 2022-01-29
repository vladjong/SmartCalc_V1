#include "smart_calc_v1.h"

Credit credit_work(CreditSting *creditString) {
  double sumCredit = 0, percentCredit = 0, termCredit = 0;
  Credit credit;
  init_credit(&credit);
  int check = covert_sting_to_double_credit(creditString, &sumCredit,
                                            &percentCredit, &termCredit);
  if (strcmp("annuity", creditString->typeCredit) == 0) {
    credit = credit_annuity_work(sumCredit, percentCredit, termCredit);
  } else if (strcmp("differentiated", creditString->typeCredit) == 0) {
    credit = credit_differentiated_work(sumCredit, percentCredit, termCredit);
  }
  credit.check = check;
  return credit;
}

int check_credit(CreditSting *creditString, double *sumCredit,
                 double *percentCredit, double *termCredit) {
  int check = SUCCESS;
  if (strcmp("year", creditString->monthOrYear) == 0) *termCredit *= 12;
  if (*termCredit > 660 || *termCredit <= 0) check = FAILURE;
  if (*percentCredit <= 0 || *percentCredit > 150) check = FAILURE;
  if (*sumCredit <= 0) check = FAILURE;
  return check;
}

int covert_sting_to_double_credit(CreditSting *creditString, double *sumCredit,
                                  double *percentCredit, double *termCredit) {
  char *ptrEnd = NULL;
  *sumCredit = strtod(creditString->sumCredit, &creditString->sumCredit);
  *percentCredit =
      strtod(creditString->percentCredit, &creditString->percentCredit);
  *termCredit = strtod(creditString->termCredit, &creditString->termCredit);
  return check_credit(creditString, sumCredit, percentCredit, termCredit);
}

void init_credit(Credit *credit) {
  credit->interestCharges = 0;
  credit->maxMonthlyPayment = 0;
  credit->minMonthlyPayment = 0;
  credit->monthlyPayment = 0;
  credit->overpayment = 0;
  credit->check = 0;
}

Credit credit_annuity_work(double sumCredit, double percentCredit,
                           double termCredit) {
  Credit annuityCredit;
  init_credit(&annuityCredit);
  double monthlyPercentRate = percentCredit / (12.0 * 100.0);
  annuityCredit.monthlyPayment =
      round(sumCredit * monthlyPercentRate /
            (1 - pow(1 + monthlyPercentRate, -termCredit)) * 100) /
      100;
  annuityCredit.interestCharges = annuityCredit.monthlyPayment * termCredit;
  annuityCredit.overpayment = annuityCredit.interestCharges - sumCredit;
  return annuityCredit;
}

Credit credit_differentiated_work(double sumCredit, double percentCredit,
                                  double termCredit) {
  Credit differentiatedCredit;
  init_credit(&differentiatedCredit);
  double totalDebt = sumCredit / termCredit;
  double monthlyPercentRate = percentCredit / (12.0 * 100.0);
  for (int numberPayment = 0; numberPayment < termCredit; numberPayment++) {
    differentiatedCredit.monthlyPayment =
        totalDebt +
        (sumCredit - totalDebt * numberPayment) * monthlyPercentRate;
    differentiatedCredit.interestCharges += differentiatedCredit.monthlyPayment;
    if (numberPayment == 0)
      differentiatedCredit.maxMonthlyPayment =
          differentiatedCredit.monthlyPayment;
    if (numberPayment == termCredit - 1)
      differentiatedCredit.minMonthlyPayment =
          differentiatedCredit.monthlyPayment;
  }
  differentiatedCredit.overpayment =
      differentiatedCredit.interestCharges - sumCredit;
  return differentiatedCredit;
}
