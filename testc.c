#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>


int main()
{
   pid_t spawnpid = fork();
   switch (spawnpid)
   {
      case -1: exit(1); break;
      case 0: exit(0); break;
      default:  break;
   }
   printf("%d\n",spawnpid);
   fflush(stdout);
}



