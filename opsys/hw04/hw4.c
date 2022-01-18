/* tcp-client.c */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#define MAXBUFFER 1024

int main(int argc, char const *argv[])
{
  setvbuf( stdout, NULL, _IONBF, 0 );

  if(argc < 4){
    perror("ERROR: Invalid argument(s)\nUSAGE: a.out <server-hostname> <server-port> <n> <int-value-1> ... <int-value-n>\n");
    return EXIT_FAILURE;
  }

  int count = atoi(argv[3]);
  int list[count + 1];
  list[0] = htonl(count);
  for(int i = 0; i < count ; i++){
    list[i + 1] = htonl(atoi(argv[i + 4]));
  }

  /* create TCP client socket (endpoint) */
  int sd = socket( AF_INET, SOCK_STREAM, 0 );
  if ( sd == -1 ) { perror( "socket() failed" ); exit( EXIT_FAILURE ); }

  /* see the man page for gethostbyname() and use it to set up
   *  the TCP server to which your client will connect
   *
   * for local testing, you should be able to use "localhost"
   */
#if 0
  struct hostent * hp = gethostbyname( "127.0.0.1" );
  struct hostent * hp = gethostbyname( "128.113.126.39" );
  struct hostent * hp = gethostbyname( "linux02.cs.rpi.edu" );
#endif

  struct hostent * hp = gethostbyname( argv[1] );  /* 127.0.0.1 */

  if ( hp == NULL )
  {
    fprintf( stderr, "ERROR: gethostbyname() failed\n" );
    return EXIT_FAILURE;
  }

  struct sockaddr_in tcp_server;
  tcp_server.sin_family = AF_INET;
  memcpy( (void *)&tcp_server.sin_addr, (void *)hp->h_addr, hp->h_length );
  unsigned short server_port = atoi(argv[2]);
  tcp_server.sin_port = htons( server_port );



  if ( connect( sd, (struct sockaddr *)&tcp_server, sizeof( tcp_server ) ) == -1 )
  {
    perror( "connect() failed" );
    return EXIT_FAILURE;
  }



  /* The implementation of the application protocol is below... */
  printf("CLIENT: Successfully connected to server\n");
  if(count == 1) printf("CLIENT: Sending %d integer value\n", count);
  else printf("CLIENT: Sending %d integer values\n", count);
  int n = write( sd, list, sizeof( list ) );    /* or use send()/recv() */
  if ( n == -1 ) { perror( "write() failed" ); return EXIT_FAILURE; }

  int result = 0;
  read(sd, &result, sizeof(int));
  result = ntohl(result);
  printf("CLIENT: Rcvd result: %d\n", result);

  char buffer[MAXBUFFER+1];
  n = read( sd, buffer, MAXBUFFER );    /* BLOCKING */


  if ( n == -1 )
  {
    perror( "read() failed" );
    return EXIT_FAILURE;
  }
  else if ( n == 0 )
  {
    printf( "CLIENT: Rcvd no data; TCP server socket was closed\n" );
  }
  else  /* n > 0 */
  {
    buffer[n] = '\0';    /* assume we rcvd text data */
    printf( "CLIENT: Rcvd secret message: \"%s\"\n", buffer );
  }

  printf( "CLIENT: Disconnected from server\n" );

  close( sd );

  return EXIT_SUCCESS;
}