#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "calcLib.h"

/* array of char* that points to char arrays. */
char *arith[] = {"add", "div", "mul", "sub", "fadd", "fdiv", "fmul", "fsub"};

/* Used for random number */
time_t myData_seedValue;

int initCalcLib(void)
{
   /* Init the random number generator with a seed, based on the current time--> should be randomish each time called */
   srand((unsigned)time(&myData_seedValue));
   return 0;
}

int initCalcLib_seed(unsigned int seed)
{
   /*
      Init the random number generator with a FIXED seed, will allow us to grab random numbers
      in the same sequence all the time. Good when debugging, bad when running live.
   */
   myData_seedValue = seed;
   srand(seed);
   return 0;
}

char *randomType(void)
{
   int Listitems = sizeof(arith) / (sizeof(char *));
   int itemPos = rand() % Listitems;
   return arith[itemPos];
}

int randomInt(void)
{
   /* Draw a random integer between 0 and 100 */
   return rand() % 100;
}

double randomFloat(void)
{
   /* Draw a random float between 0.0 and 100.0 */
   double x = (double)rand() / (double)(RAND_MAX / 100.0);
   return x;
}

int add(int a, int b)
{
   return a + b;
}

int sub(int a, int b)
{
   return a - b;
}

int mul(int a, int b)
{
   return a * b;
}

int intDiv(int a, int b)
{
   if (b == 0)
      return 0;  // Handle division by zero
   return a / b; // Integer division (truncates)
}

double floatAdd(double a, double b)
{
   return a + b;
}

double floatSub(double a, double b)
{
   return a - b;
}

double floatMul(double a, double b)
{
   return a * b;
}

double floatDiv(double a, double b)
{
   if (b == 0.0)
      return 0.0; // Handle division by zero
   return a / b;
}