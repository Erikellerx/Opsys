#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>


//check if all the char are alpha
int is_all_alpha(char * word){
	for(int i = 0; i < strlen(word); i++){
		if (! isalpha(* (word + i) )) return 0;
	}
	return 1;
}

//check if all the car are int
int is_all_digit(char * word){
	for(int i = 0; i < strlen(word); i++){
		if( ! isdigit(*(word + i))) return 0;
	}
	return 1;
}

//hash the word
int hash(char * word , int size){
	if(is_all_alpha(word)){
		int output = 0;
		for(int i = 0; i < strlen(word); i++){
			output += (int) *(word + i);
		}
		return output % size;
	}else{
		return atoi(word) % size;
	}
	
}


int main(int argc, char  **argv)
{
		
	setvbuf( stdout, NULL, _IONBF, 0 );
	//error input
	if (argc < 3){
		fprintf(stderr, "%s", "ERROR: <Invalid input>\n");
		return EXIT_FAILURE;
	}	

	int cache_size = atoi(*(argv + 1));
	if (cache_size < 1) {
		fprintf(stderr, "%s", "ERROR: <Invalid cache size>\n");
		return EXIT_FAILURE;
	}
	//alloc buffer and cache
	char * buffer = calloc(128 , sizeof(char));
	void ** cache = calloc(cache_size , sizeof(void * ));
	int * type = calloc(cache_size , sizeof(int)); // keep record which type stored
	char temp = '\0'; //read each char
	int count = 0;
	int mark = 0;
	//initialize the type 0:char 1:int
	for (int i = 0; i < cache_size; i++) 
		*(type + i) = -1;

	for(int i = 2; i < argc ; i++){
		//open the file
		int fd = open(*(argv + i),O_RDONLY );
		if (fd == -1){
			perror( "ERROR: <Failed to open the file>\n" );
			return EXIT_FAILURE;
		}

		//read each 
		while(read(fd , &temp , 1)){
			//check valid char
			if((isalpha(temp) || isdigit(temp))){
				//valid char but different type
				if((isalpha(temp) && ! isalpha(*buffer)) || (isdigit(temp) && !isdigit(*buffer))){
					mark = 1;
				}else{
					//load char into buffer
					*(buffer + count) = temp;
					count ++;					
				}
			
			}
			//encounter the different type or invalid char: print the result
			if(mark == 1 || (!isdigit(temp) && !isalpha(temp))) {
				*(buffer + count) = '\0';	
				int hash_num = hash(buffer , cache_size);
				//print if type is word
				if(is_all_alpha(buffer) && strlen(buffer) >= 3){
					*(type + hash_num) = 0;
					//allocate
					if (*(cache + hash_num) == NULL){
						printf("Word \"%s\" ==> %d (calloc)\n" , buffer , hash_num);
						*(cache + hash_num) = calloc(count + 1 , sizeof(char));
						strcpy(*(cache + hash_num) , buffer);

					}else{//reallocate
						printf("Word \"%s\" ==> %d (realloc)\n" , buffer ,hash_num);
						*(cache + hash_num) = (char*) realloc( * (cache + hash_num) , count + 1);
						strcpy(*(cache + hash_num)  , buffer);
					}
				}
				//print if type is Int
				else if(is_all_digit(buffer) && strlen(buffer) > 0){
					*(type + hash_num) = 1;
					//allocate
					if( * (cache + hash_num) == NULL){
						printf("Integer \"%s\" ==> %d (calloc)\n" , buffer , hash_num);
						* (cache + hash_num)= calloc(1, sizeof(int));
						**((int **)(cache + hash_num)) = atoi(buffer);
						
					}else{//reallocate
						printf("Integer \"%s\" ==> %d (realloc)\n" , buffer ,hash_num );
						*(cache + hash_num) = (int *) realloc( *(cache + hash_num), sizeof(int));
						**((int **)(cache + hash_num)) = atoi(buffer);

					}
				}
				//reset the buffer
				memset(buffer, 0, 128);
				count = 0;
				//load the next char if next char is still valid
				if(mark == 1){
					*(buffer) = temp;
					count ++;
					mark = 0;
				}
			}
		}
		close(fd);
	}


	//print the entire cache	

	for(int i = 0; i < cache_size ; i ++){
		if(*(type + i) == 1)
			printf("Cache index %d ==> int %d\n" , i , **((int **)(cache + i)));
		else if (*(type + i) == 0)
			printf("Cache index %d ==> \"%s\"\n" , i , (char *)*(cache + i));
	}
	//clear the memory
	for(int i = 0; i < cache_size; i ++){
		free(*(cache + i));
	}
	free(cache);
	free(type);
	free(buffer);

	return 0;
}