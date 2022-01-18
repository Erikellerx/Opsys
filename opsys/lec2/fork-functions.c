#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int lecex2_child( int n ){

	char c = '\0';
	int fd = open("data.txt" , O_RDONLY);
	if(fd == -1){
		perror("ERROR: File not found\n");
		abort();
	}
	for(int i = 0; i < n ; i ++){
		if(! read(fd , &c , 1)) abort();
	}

	return (int) c;
}

int lecex2_parent(){

	int status;
	wait(&status);

	if(WIFSIGNALED(status)){
		printf("PARENT: child process terminated abnormally!\n");
		perror("PARENT: child process terminated abnormally!\n");
		return EXIT_FAILURE;
		
	}else{
		printf("PARENT: child process returned \'%c\'\n",WEXITSTATUS(status));
		return 0;
	}

}