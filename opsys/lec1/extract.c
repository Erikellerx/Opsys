#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
////li ruohua
////!!!!!!!!!!!!!!!!


int main(int argc, char  **argv)
{	
	
	char c = '\0';
	int fd = open(*(argv + 2 ),O_RDONLY);
	int num = atoi(*(argv + 1));
	lseek(fd , num - 1 , SEEK_CUR);

	while ( read(fd , &c , 1)){	

		lseek(fd , num - 1 , SEEK_CUR);	
		printf("%c",c);	

	}
	printf("\n");
	close(fd);
	return 0;
}
