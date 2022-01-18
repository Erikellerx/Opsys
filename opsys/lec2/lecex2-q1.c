#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char const *argv[])
{
	setvbuf( stdout, NULL, _IONBF, 0 );
	pid_t p;

	p = fork();

	if(p == -1){
		perror("fork() failed");
		return EXIT_FAILURE;
	}

	if(p > 0){
		printf("PARENT: start here!\n");

		int status;
		waitpid(p , &status , 0);

		printf("PARENT: child process terminated.\n");
		printf("PARENT: sigh, i'm gonna miss that little child process.\n");

	}
	else if (p == 0){
		printf("CHILD: happy birthday to me!\n");
		printf("CHILD: idk......good-bye!\n");
	}

	
	return 0;
}