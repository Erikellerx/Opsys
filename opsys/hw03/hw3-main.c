/* hw3-main.c (v1.1) */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int next_thread_id;        /* initialize to 1 */

int max_squares;           /* initialize to 0 */

char *** dead_end_boards;  /* initialize as array of NULL pointers of size 8 */

/* write the simulate() function and place all of your code in hw3.c */
int simulate( int argc, char * argv[] );

int main( int argc, char * argv[] )
{
  next_thread_id = 1;
  max_squares = 0;
  dead_end_boards = calloc( 8, sizeof( char ** ) );
  if ( dead_end_boards == NULL ) { perror( "calloc() failed" ); return EXIT_FAILURE; }
  int rc = simulate( argc, argv );

  /* on Submitty, there will be more code here that validates the
      global variables at this point... */

  printf("next_thread_id: %d\n", next_thread_id);

  int m = atoi( argv[1] );
  int n = atoi( argv[2] );

  int dead_end_boards_index = 0;  /* this is set per each test case */

  for ( int i = 0 ; i < dead_end_boards_index ; i++ )
  {
    for ( int j = 0 ; j < m ; j++ ) free( dead_end_boards[i][j] );
    free( dead_end_boards[i] );
  }

  free( dead_end_boards );

  return rc;
}