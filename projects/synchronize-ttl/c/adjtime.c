#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char * argv[])
{
  struct timeval oldtime, newtime;
  int status;

  if (argc<2) {
    printf("ERROR: Use as %s <sec> <usec>\n", argv[0]);
    return 1;
  }


  newtime.tv_sec = atol(argv[1]);
  newtime.tv_usec = atol(argv[2]);

  status = adjtime(&newtime, &oldtime);

  printf("status = %d\n", status);
  printf("oldtime.tv_sec  = %ld\n", oldtime.tv_sec);
  printf("oldtime.tv_usec = %06ld\n", oldtime.tv_usec);
  printf("newtime.tv_sec  = %ld\n", newtime.tv_sec);
  printf("newtime.tv_usec = %06ld\n", newtime.tv_usec);

  return 0;
}
