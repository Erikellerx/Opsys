#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
//#include "lecex3-q2-copy-file.c"

void * copy_file( void * arg );

int main(int argc, char  *argv[])
{
	
	pthread_t threads[argc];

	for(int i = 1; i < argc; i ++){

		printf("MAIN: Creating thread to copy \"%s\"\n", argv[i]);
		int rc = pthread_create(&threads[i], NULL, copy_file, argv[i]);

		if ( rc != 0 )
      	{
        fprintf( stderr, "ERROR: pthread_create() failed (%d): %s\n",
                 rc, strerror( rc ) );
        return EXIT_FAILURE;
      	}

	}

	int total = 0;
	for(int i = 1; i < argc; i ++){

		void * retval;
		pthread_join(threads[i] , &retval);
		printf("MAIN: Thread completed copying %d bytes for \"%s\"\n",*(int*)retval,argv[i] );
		total += *(int*)retval;

	}

	if(argc - 1 == 1)
	printf("MAIN: Successfully copied %d bytes via %d child thread\n", total, argc-1);
	else 
	printf("MAIN: Successfully copied %d bytes via %d child threads\n", total, argc-1);

	return 0;
}