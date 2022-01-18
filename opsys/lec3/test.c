/* alarm-v3.c */

/* implementation using multiple threads */

/* A multithreaded process is a single process that has
    a "main thread" of execution and other "threads of execution"
     that all SHARE the encompassing process's address space, variables,
      fd table, instructions to execute, etc. */

/* gcc -Wall alarm-v3.c -pthread */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define MAX_LINE 80

/* definition of a struct to send to each thread,
    because the thread function can only accept one arg, i.e., void *
 */
typedef struct
{
  int seconds;
  char msg[MAX_LINE];   /* TO DO: use char* msg here and calloc()... */
} alarm_t;

/* function executed by each thread */
void * alarm_thread_function( void * arg )
{
  alarm_t * alarm = (alarm_t *)arg;

  pthread_t mytid = pthread_self();   /* get my thread ID */

  /* pthread_detach() enables pthreads to reclaim the
      thread resources when this thread terminates
       (i.e., do not leave a "zombie thread" behind) */
  pthread_detach( mytid );
  /* we therefore do not need to call pthread_join() in
      this child thread anymore ... */
  /* TO DO: check the return code from pthread_detach() */

  printf( "Alarm set for %d seconds\n", alarm->seconds );
  sleep( alarm->seconds );
  printf( "ALARM (%d): %s\n", alarm->seconds, alarm->msg );

  free( alarm );

  return NULL;
}


int main()
{
  const int maxline = MAX_LINE;
  char * line = calloc( maxline, sizeof( char ) );
  char * msg = calloc( maxline, sizeof( char ) );

  char * format_string = calloc( maxline, sizeof( char ) );
  sprintf( format_string, "%%d %%%d[^\n]", maxline - 1 );

  while ( 1 )
  {
    int seconds;

    printf( "Set alarm (<sec> <msg>) -- 0 to exit: " );

    if ( fgets( line, maxline - 1, stdin ) == NULL ) return EXIT_FAILURE;

    if ( strlen( line ) <= 1 ) continue;   /* skip blank lines */

    if ( sscanf( line, format_string, &seconds, msg ) < 2 || seconds < 0 )
    {
      if ( seconds == 0 ) break;
      fprintf( stderr, "ERROR: invalid alarm request\n" );
    }
    else
    {
      pthread_t tid;   /* thread ID */

      /* dynamically allocate memory for the next alarm struct */
      alarm_t * alarm = malloc( sizeof( alarm_t ) );
      alarm->seconds = seconds;
      strcpy( alarm->msg, msg );

      /* create a child thread... */
      int rc = pthread_create( &tid, NULL, alarm_thread_function, alarm );
                                      /*   ^^^^^^^^^^^^^^^^^^^^^  ^^^^^
                                            |                         |
                                          the function each thread    |
                                           will execute once the      |
                                            thread is up and running  |
                                                                      |
                                                          pointer to the input
                                                       to the thread func */
      if ( rc != 0 )
      {
        fprintf( stderr, "ERROR: pthread_create() failed (%d): %s\n",
                 rc, strerror( rc ) );
        return EXIT_FAILURE;
      }


      usleep( 1000 );

    }
  }

  free( msg );
  free( line );
  free( format_string );

  return EXIT_SUCCESS;
}
{"mode":"full","isActive":false}