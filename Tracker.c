#include <math.h> // for round()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h> // for system("cls")
#include <direct.h>
#define rmdir _rmdir
#define mkdir(path, mode) _mkdir(path)
#define CLEAR system("cls")
#else
#include <sys/stat.h> // for mkdir
#include <unistd.h>   // for rmdir
#define CLEAR system("clear")
#endif
#define DATA_INFO "Appdata"
#define MONTHLY_INFO "Monthly_info"
#define MONTHS_DIR "Months"
#define CURRENT_BALANCE "Current Balance"
#define MONTHLY_EXPENSES "Monthly Expenses"
#define DATA_MONTHS DATA_INFO "/" MONTHS_DIR      // Appdata/Months
#define MONTHLY_PATH DATA_MONTHS "/" MONTHLY_INFO // Appdata/Months/Monthly_info

#define MaxMonths 6

void pressEnterToContinue() {
  printf("Press Enter to continue...");
  getchar(); // wait for Enter
}

void ClearBuffer() {
  // For Integers
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ; // Flush stdin
}

void RemoveLine(char *str) { // For strings
  str[strcspn(str, "\n")] = 0;
}

void Spacer(int lineSize) {
  for (int i = 1; i <= lineSize - 2; i++) {
    putchar('*');
  }
  putchar('\n');
}

int CurrentDate() {
  time_t now;
  struct tm *t;

  time(&now);
  t = localtime(&now);

  return t->tm_mday;
}

void deleteMonthFolder(const char *monthName) {
  char path1[128], path2[128], folder[128];

  // Use \\ for Windows-style paths (or /, both work in practice)
  snprintf(path1, sizeof(path1), "%s/%s/%s %s", DATA_MONTHS, monthName,
           monthName, CURRENT_BALANCE);
  snprintf(path2, sizeof(path2), "%s/%s/%s %s", DATA_MONTHS, monthName,
           monthName, MONTHLY_EXPENSES);
  snprintf(folder, sizeof(folder), "%s/%s", DATA_MONTHS, monthName);

  // Delete files
  remove(path1);
  remove(path2);

  // Delete folder (must be empty)
  rmdir(folder);
}

void StartUp(char *monthBuffer, size_t monthBufferSize, char *workingBuffer,
             size_t workingBufferSize, char *expenseBuffer,
             size_t expenseBufferSize) {

  FILE *limitCheck = fopen(MONTHLY_PATH, "r");
  if (limitCheck != NULL) {
    int counter = 0;
    char lines[MaxMonths][100];

    while (fgets(lines[counter], 100, limitCheck)) {
      RemoveLine(lines[counter]);
      counter++;
    }
    fclose(limitCheck);

    if (counter > 6) {
      int trim = counter - 6;

      for (int i = 0; i < trim; i++) {
        char monthName[30];
        sscanf(lines[i], "%29[^,]", monthName);
        deleteMonthFolder(monthName);
      }

      // Shift the remaining 6 lines down
      for (int i = 0; i < 6; i++) {
        strcpy(lines[i], lines[trim + i]);
      }
      counter = 6;
    }

    FILE *out = fopen(MONTHLY_PATH, "w");
    for (int i = 0; i < counter; i++) {
      fprintf(out, "%s\n", lines[i]);
    }
    fclose(out);
  }

  mkdir(DATA_INFO, 0755);
  mkdir(DATA_MONTHS, 0755);
  int Choice;
  int monthChoice;
  do {
    printf("What would you like to do?\n1- Create a New Month\n2- Select a "
           "Month\n-");
    scanf("%i", &Choice);
    ClearBuffer();
    CLEAR;
    if (Choice == 1) {
      float startingBal;
      float income;
      float budgetMargin = 20.0;
      char buffmonth[20];
      char pathMonths[50];
      char fullPath[100];

      printf("What would you like to call it? E.g. Nov 24\n- ");
      fgets(buffmonth, sizeof(buffmonth), stdin);
      RemoveLine(buffmonth);
      snprintf(pathMonths, sizeof(pathMonths), "%s/%s", DATA_MONTHS, buffmonth);
      snprintf(fullPath, sizeof(fullPath), "%s/%s %s", pathMonths, buffmonth,
               CURRENT_BALANCE); // makes path to appData/Months/{buffmonth}+"
                                 // Current balance" to fullPath
      snprintf(expenseBuffer, expenseBufferSize, "%s/%s %s", pathMonths,
               buffmonth, MONTHLY_EXPENSES);
      printf("What is your starting balance?\n-");
      scanf("%f", &startingBal);
      ClearBuffer();
      startingBal = roundf(startingBal * 100) / 100;
      printf("What is your Previous income?\n-");
      scanf("%f", &income);
      ClearBuffer();
      income = roundf(income * 100) / 100;
      mkdir(pathMonths, 0755); // Makes a path from appData/months/[month name]

      FILE *balf;
      FILE *monf;

      monf =
          fopen(MONTHLY_PATH, "a"); // goes to path appData/Months/Monthly_info
      fprintf(monf, "%s,%.2f,%.2f,%.1f\n", buffmonth, startingBal, income,
              budgetMargin);
      fclose(monf);
      // FILE *fptr;
      balf = fopen(fullPath, "a");
      fprintf(balf, "%.2f", startingBal);
      fclose(balf);
      printf("Starting Balance stored - %.2f\n", startingBal);
      snprintf(monthBuffer, monthBufferSize, "%s", buffmonth);
      snprintf(workingBuffer, workingBufferSize, "%s", fullPath);
      pressEnterToContinue();
    } else if (Choice == 2) {
      char line[100];
      int month_index;
      char monthName[MaxMonths][20];
      float loaded_starting_balance[MaxMonths],
          loaded_previous_income[MaxMonths], loaded_budget_margin[MaxMonths];
      do {
        month_index = 0;
        FILE *fptr = fopen(MONTHLY_PATH, "r");
        if (fptr == NULL) {
          printf("Could not open file.\n");
          pressEnterToContinue();
          break;
        }

        while (fgets(line, sizeof(line), fptr) && month_index < MaxMonths) {
          sscanf(line, "%19[^,], %f[^,], %f[^,], %f[^\n]",
                 monthName[month_index], &loaded_starting_balance[month_index],
                 &loaded_previous_income[month_index],
                 &loaded_budget_margin[month_index]);
          printf("%i. %s\n", month_index + 1, monthName[month_index]);
          month_index++;
        }
        fclose(fptr);
        printf("Please pick an option\n-");
        scanf("%i", &monthChoice);
        ClearBuffer();
        monthChoice--;

        if (monthChoice >= 0 && monthChoice <= month_index - 1) {
          strcpy(monthBuffer, monthName[monthChoice]);
          snprintf(workingBuffer, workingBufferSize, "%s/%s/%s %s", DATA_MONTHS,
                   monthBuffer, monthBuffer, CURRENT_BALANCE);
          snprintf(expenseBuffer, expenseBufferSize, "%s/%s/%s %s", DATA_MONTHS,
                   monthBuffer, monthBuffer, MONTHLY_EXPENSES);

        } else {
          CLEAR;
          printf("Try again\n");
        }
      } while (monthChoice < 0 ||
               monthChoice >
                   month_index -
                       1); // the minus one makes it so it ignores the \n

    }

    else {
      printf("Try again\n");
    }
  } while (Choice != 1 && Choice != 2);
}

void UpdateBalance(char *workingBuff) {
  float upBal;
  char correct;
  do {
    printf("Updated Balance: ");
    scanf("%f", &upBal);
    ClearBuffer();
    upBal = roundf(upBal * 100) / 100;
    printf("Is this correct? : %.2f\ny/n: ", upBal);
    scanf("%c", &correct);
    ClearBuffer();
    if (correct == 'y' || correct == 'Y') {
      FILE *chgptr = fopen(workingBuff, "w");
      fprintf(chgptr, "%.2f", upBal);
      fclose(chgptr);
    }
  } while (correct != 'y' && correct != 'Y');
}

void MonthlyExpenses(char *monthName) {
  int monthlyChoice;
  int expenseChoice;
  char monthPath[60];
  snprintf(monthPath, sizeof(monthPath), "%s/%s/%s %s", DATA_MONTHS, monthName,
           monthName, MONTHLY_EXPENSES);
  do {
    printf("Monthly Management Menu\n");
    Spacer(25);
    printf("1- Add a Monthly Expense\n2- Delete a Monthly Expense\n3- View "
           "Monthly Expenses\n4- Exit\n-");
    scanf("%d", &monthlyChoice);
    ClearBuffer();
    CLEAR;
    if (monthlyChoice == 1) {
      printf("1- Add an Expense\n2- Copy from last months expenses\n-");
      scanf("%i", &expenseChoice);
      ClearBuffer();
      if (expenseChoice == 1) {

        char expenseName[40];
        float expenseAmount;
        int DOP;

        printf("Name of Expense:\n-");
        fgets(expenseName, sizeof(expenseName), stdin);
        RemoveLine(expenseName);

        printf("Amount:\n-");
        scanf("%f", &expenseAmount);
        ClearBuffer();
        expenseAmount = roundf(expenseAmount * 100) / 100;

        printf("Date of Payment: (DD)\n-");
        scanf("%i", &DOP);
        ClearBuffer();

        FILE *monthFile = fopen(monthPath, "a");
        fprintf(monthFile, "%s,%.2f,%i\n", expenseName, expenseAmount, DOP);
        fclose(monthFile);
      }

      else if (expenseChoice == 2) {
        char buffer[100];
        char monthNames[MaxMonths][20];
        char monthCopyPath[60];
        int line = 0;
        int counter = 1;

        int monthChoice = 0;
        char pickedMonth[20];

        printf("What month would you like to copy from:\n");
        FILE *monthlyInfo = fopen(MONTHLY_PATH, "r");
        while (fgets(buffer, sizeof(buffer), monthlyInfo)) {
          RemoveLine(buffer);
          sscanf(buffer, "%19[^,]", monthNames[line]);

          if (strcmp(monthName, monthNames[line]) == 0) {
            continue;
          }
          printf("%i- Name: %s\n", counter, monthNames[line]);
          counter++;
          line++;
        }
        fclose(monthlyInfo);
        putchar('-');
        scanf("%i", &monthChoice);
        ClearBuffer();
        strcpy(pickedMonth, monthNames[monthChoice - 1]);
        snprintf(monthCopyPath, sizeof(monthCopyPath), "%s/%s/%s %s",DATA_MONTHS, pickedMonth, pickedMonth, MONTHLY_EXPENSES);
        if (strcmp(monthPath, monthCopyPath) == 0) {
          printf("File paths are the same!!!\n");
          pressEnterToContinue();
          break;
        }

        FILE *monthExchange = fopen(monthPath, "w");
        FILE *copiedMonth = fopen(monthCopyPath, "r");

        while (fgets(buffer, sizeof(buffer), copiedMonth)) {
          fprintf(monthExchange, "%s", buffer);
        }
        fclose(copiedMonth);
        fclose(monthExchange);

        printf("Copy complete!\n");
        pressEnterToContinue();
      }
    } else if (monthlyChoice == 2) {
      char **lines;
      int lineCount = 0;
      int lineCapacity = 10;

      lines = malloc(lineCapacity * sizeof(char *));

      FILE *monthDelete = fopen(monthPath, "r");
      char buffer[255];
      char nameBuffer[20][20];

      while (fgets(buffer, sizeof(buffer), monthDelete)) {
        RemoveLine(buffer);

        if (lineCount >= lineCapacity) {
          lineCapacity *= 2;
          lines = realloc(lines, lineCapacity * sizeof(char *));
        }

        sscanf(buffer, "%19[^,]", nameBuffer[lineCount]);
        lines[lineCount] = malloc(strlen(buffer) + 1);
        strcpy(lines[lineCount], buffer);
        printf("%i. %s\n", lineCount + 1, nameBuffer[lineCount]);
        lineCount++;
      }
      fclose(monthDelete);
      int deleteIndex = 0;
      printf("Month Deletion: 0 - Exit\n-");
      scanf("%i", &deleteIndex);
      ClearBuffer();
      if (deleteIndex == 0) {
        for (int i = 0; i < lineCount; i++) {
          free(lines[i]);
        }
        free(lines);
        CLEAR;
        continue;
      }
      deleteIndex--;

      if (deleteIndex >= 0 && deleteIndex < lineCount) {
        free(lines[deleteIndex]); // Free memory for the line
        for (int i = deleteIndex; i < lineCount - 1; i++) {
          lines[i] = lines[i + 1]; // Shift pointers
        }
        lineCount--;
      }
      FILE *monthRenew = fopen(monthPath, "w");
      for (int i = 0; i < lineCount; i++) {
        fprintf(monthRenew, "%s\n", lines[i]);
      }
      fclose(monthRenew);
      for (int i = 0; i < lineCount; i++) {
        free(lines[i]);
      }
      free(lines);

    } else if (monthlyChoice == 3) {
      FILE *monthView = fopen(monthPath, "r");

      char buffer[255];
      char nameBuffer[20];
      float amountBuffer = 0.00;
      int dateBuffer = 0;
      float total = 0.00;

      while (fgets(buffer, sizeof(buffer), monthView)) {
        RemoveLine(buffer);
        sscanf(buffer, "%19[^,], %f, %i", nameBuffer, &amountBuffer,
               &dateBuffer);
        printf("Name: %s Amount: %.2f Date of Payment: %i\n", nameBuffer,
               amountBuffer, dateBuffer);
        total += amountBuffer;
      }
      fclose(monthView);
      printf("Total: %.2f\n", total);
      pressEnterToContinue();
    } else if (monthlyChoice == 4) {
      printf("closing");

    } else {
    }
    CLEAR;

  } while (monthlyChoice != 4);
}

void BudgetControl(char *Listname, char *workingPath, char *expensePath) {
  char buffer[100];
  char dummyVar[50];
  int day = CurrentDate();
  int marginChoice = 0;
  do {
    float currentExpense = 0.00;
    float totalExpense = 0.00;
    float totalExpenseBuffer = 0.00;
    int dateOfExpense = 0;

    char monthName[20];
    float startingBalance = 0.00;
    float currentBalance = 0.00;
    float income = 0.00;
    float incomeAfterExpenses = 0.00;
    float margin = 0.0;

    float totalBudget = 0.00;
    float incomeSaving = 0.00;
    float moneyRemaining = 0.00;
    float dailySpending = 0.00;
    float excessSpending = 0.00;
    float remainingBudget = 0.00;

    float newMargin = 0.00;

    FILE *balFind = fopen(workingPath, "r");
    fscanf(balFind, "%f", &currentBalance);
    fclose(balFind);

    FILE *monthsExpenses = fopen(expensePath, "r");
    if (monthsExpenses != NULL) {
      while (fgets(buffer, sizeof(buffer), monthsExpenses)) {
        RemoveLine(buffer);
        sscanf(buffer, "%49[^,], %f, %i", dummyVar, &totalExpenseBuffer,
               &dateOfExpense);
        totalExpense += totalExpenseBuffer;

        if (dateOfExpense <= day) {
          currentExpense += totalExpenseBuffer;
        }
      }
      fclose(monthsExpenses);
    }

    printf("Budget Management System\n");
    Spacer(26);

    FILE *monthData = fopen(MONTHLY_PATH, "r");
    if (monthData != NULL) {

      while (fgets(buffer, sizeof(buffer), monthData)) {
        RemoveLine(buffer);
        sscanf(buffer, "%19[^,], %f, %f, %f", monthName, &startingBalance,
               &income, &margin);

        if (strcmp(monthName, Listname) == 0) {

          incomeAfterExpenses = income - totalExpense;
          currentExpense = (startingBalance - currentBalance) - currentExpense;
          incomeSaving = (margin / 100) * income;
          totalBudget = incomeAfterExpenses - incomeSaving;
          moneyRemaining = startingBalance - totalExpense - totalBudget;
          dailySpending = totalBudget / 31;
          excessSpending = (dailySpending * day) - currentExpense;
          remainingBudget = totalBudget - currentExpense;

          printf("%i\n", day);

          printf("Starting Balance: %.2f\n", startingBalance);
          printf("Current Balance:  %.2f\n", currentBalance);
          printf("Income: %.2f\n", income);
          printf("Usable Income: %.2f\n", incomeAfterExpenses);
          printf("Margin: %.1f\n", margin);
          printf("Money Remaining: %.2f\n", moneyRemaining);
          Spacer(30);
          printf("Income Saving: %.2f\n", incomeSaving);
          printf("Total Budget: %.2f\n", totalBudget);
          printf("Remaining budget: %.2f\n", remainingBudget);
          printf("Total Spent: %.2f\n", currentExpense);
          Spacer(30);
          printf("Daily: %.2f\n", dailySpending);
          printf("Excess Spending: %.2f\n", excessSpending);

          marginChoice = 0;
          printf("\n\n1- Change Margin\n2- Exit\n-");
          scanf("%i", &marginChoice);
          ClearBuffer();
          if (marginChoice == 1) {
            printf("New Margin: ");
            scanf("%f", &newMargin);
            ClearBuffer();
            newMargin = roundf(newMargin * 10) / 10;

            FILE *changeMargin = fopen(MONTHLY_PATH, "r");
            char lines[MaxMonths][100];
            int index = 0;

            while (fgets(lines[index], 100, changeMargin)) {
              RemoveLine(lines[index]);
              index++;
            }
            fclose(changeMargin);

            for (int i = 0; i < index; i++) {
              char month[20];
              float startingBal, prevIncome, margin;

              sscanf(lines[i], "%19[^,],%f,%f,%f", month, &startingBal,
                     &prevIncome, &margin);
              if (strcmp(month, Listname) == 0) {
                snprintf(lines[i], 100, "%s,%.2f,%.2f,%.1f", month, startingBal,
                         prevIncome, newMargin);
                break;
              }
            }

            FILE *marginChanged = fopen(MONTHLY_PATH, "w");
            for (int i = 0; i < index; i++) {
              fprintf(marginChanged, "%s\n", lines[i]);
            }
            fclose(marginChanged);
            continue;
          }

          else if (marginChoice == 2) {
            break;
          }
        }
      }
      fclose(monthData);
    } else {
      printf("Could not open file.\n");
      pressEnterToContinue();
    }
    CLEAR;
  } while (marginChoice != 2);
}

int main() {
  CLEAR;
  char currentMonth[100]; // The File name we're on
  char workingMonth[200]; // The path to the month Current Balance we're working
                          // on
  char expenseMonth[200]; // The path to the Month's Expenses

  CurrentDate();

  StartUp(currentMonth, sizeof(currentMonth), workingMonth,
          sizeof(workingMonth), expenseMonth, sizeof(expenseMonth));
  CLEAR;
  printf("Selected Month - %s\n", currentMonth);

  int mainChoice;
  do {
    FILE *monthprt = fopen(workingMonth, "r");
    if (monthprt == NULL) {
      printf("Could not open file.\n");
    }
    float currentBalance;
    fscanf(monthprt, "%f", &currentBalance);
    fclose(monthprt);

    CLEAR;
    printf("Working month: %s\n", currentMonth);
    printf("Current Balance: %.2f\n", currentBalance);
    printf("Main menu\n");
    Spacer(11);
    printf("1 - Update Balance\n2 - Monthly Expenses\n3 - Budget Control\n4 - "
           "change Month\n0 - Exit\n-");
    scanf("%i", &mainChoice);
    ClearBuffer();
    CLEAR;
    if (mainChoice == 1) {
      UpdateBalance(workingMonth);
      FILE *monthprt = fopen(workingMonth, "r");
      if (monthprt == NULL) {
        printf("Could not open file.\n");
      }
      fscanf(monthprt, "%f", &currentBalance);
      fclose(monthprt);
    }

    else if (mainChoice == 2) {
      MonthlyExpenses(currentMonth);
    }

    else if (mainChoice == 3) {
      BudgetControl(currentMonth, workingMonth, expenseMonth);
    }

    else if (mainChoice == 4) {
      StartUp(currentMonth, sizeof(currentMonth), workingMonth,
              sizeof(workingMonth), expenseMonth, sizeof(expenseMonth));
      CLEAR;
    }
    if (mainChoice > 4) {
      CLEAR;
      printf("Please Pick a Numbers shown.\n");
      pressEnterToContinue();
    }
  } while (mainChoice != 0);
  return 0;
}
