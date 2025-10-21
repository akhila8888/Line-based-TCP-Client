#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __CALC_LIB
#define __CALC_LIB

  int initCalcLib(void);                   // Init internal variables to the library, if needed.
  int initCalcLib_seed(unsigned int seed); // Init internal variables to the library, use <seed> for specific variable.

  char *randomType(void);   // Return a string to a mathematical operator
  int randomInt(void);      // Return a random integer, between 0 and 100.
  double randomFloat(void); // Return a random float between 0.0 and 100.0

  // Arithmetic functions
  int add(int a, int b);               // Integer addition
  int sub(int a, int b);               // Integer subtraction
  int mul(int a, int b);               // Integer multiplication
  int intDiv(int a, int b);            // Integer division (truncate)
  double floatAdd(double a, double b); // Floating-point addition
  double floatSub(double a, double b); // Floating-point subtraction
  double floatMul(double a, double b); // Floating-point multiplication
  double floatDiv(double a, double b); // Floating-point division

#endif

#ifdef __cplusplus
}
#endif