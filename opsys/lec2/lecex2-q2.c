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

		pid_t p = fork();

		if(p == 0){
			printf("CHILD B: and happy birthday to me!\n");
			printf("CHILD B: nothing to do.\n");
		}else if (p > 0){
			waitpid(p , &status , 0);
			printf("PARENT: both child processes terminated.\n");
			printf("PARENT: end here!\n");
		}

	}
	else if (p == 0){
		printf("CHILD A: happy birthday to me!\n");
		printf("CHILD A: idk......good-bye!\n");
	}

	
	return 0;
}