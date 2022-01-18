#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

int lecex3_q1_child( int pipefd ){

	key_t key;
	int size;

	read(pipefd , &key, sizeof(key_t));
	read(pipefd, &size, sizeof(int));

	int shmid = shmget(key, size , IPC_CREAT);

	if ( shmid == -1 )
  	{
    	perror( "shmget() failed" );
	    return EXIT_FAILURE;
	 }

	 char * data = calloc(size , sizeof(char));
	 data = shmat(shmid, NULL, 0);

	 for(int i = 0; i < size; i ++){

	 	*(data + i) = toupper(*(data + i));

	 }

	 int rc = shmdt( data );
	 if ( rc == -1 )
	 {
	    perror( "shmdt() failed" );
	    exit( EXIT_FAILURE );
	 }


	 return EXIT_SUCCESS;


}