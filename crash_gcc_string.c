#include <stdio.h>
#include <stdlib.h>

#define TEST "This is a test, test, test, test, test, test\n"
#define TEST2 TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
#define TEST3 TEST2 TEST2 TEST2 TEST2 TEST2 TEST2 TEST2 TEST2 TEST2 TEST2
#define TEST4 TEST3 TEST3 TEST3 TEST3 TEST3 TEST3 TEST3 TEST3 TEST3 TEST3
#define TEST5 TEST4 TEST4 TEST4 TEST4 TEST4 TEST4 TEST4 TEST4 TEST4 TEST4 
#define TEST6 TEST5 TEST5 TEST5 TEST5 TEST5 TEST5 TEST5 TEST5 TEST5 TEST5
#define TEST7 TEST6 TEST6 TEST6 TEST6 TEST6 TEST6 TEST6 TEST6 TEST6 TEST6
#define TEST8 TEST7 TEST7 TEST7 TEST7 TEST7 TEST7 TEST7 TEST7 TEST7 TEST7 
#define TEST9 TEST8 TEST8 TEST8 TEST8 TEST8 TEST8 TEST8 TEST8 TEST8 TEST8
#define TEST10 TEST9 TEST9 TEST9 TEST9 TEST9 TEST9 TEST9 TEST9 TEST9 TEST9
#define TEST11 TEST10 TEST10 TEST10 TEST10 TEST10 TEST10 TEST10 TEST10 TEST10
#define TEST12 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11 TEST11
#define TEST13 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12 TEST12
#define TEST14 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 TEST13 
#define TEST15 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14 TEST14
#define TEST16 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 TEST15 

int main() {
  printf(TEST5 "\0");
  return 0;
}