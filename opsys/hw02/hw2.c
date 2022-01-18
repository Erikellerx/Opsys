#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>


int parse_path_funct(char ** all_path){

	char * MYPATH = calloc(1024 , sizeof(char));
	if(getenv("MYPATH") != NULL) strcpy(MYPATH , getenv("MYPATH"));
	else strcpy(MYPATH , "/bin:.");

	char * path = strtok(MYPATH, ":");
	int path_count = 0;
	
	//parse all the path and load into all_path
	while(path != NULL){
		*(all_path + path_count) = calloc(100, sizeof(char));
		strcpy(*(all_path + path_count) , path);
		*(*(all_path + path_count) + strlen(*(all_path + path_count))) = '\0';
		path_count += 1;
		path = strtok(NULL , ":");
	}
	free(MYPATH);
	return path_count;
}

int parse_input_funct(char ** parsed_input, char* user_input){
//parse the command line
		
	int input_count = 0;
	char * each_param = strtok(user_input , " ");

	while(each_param != NULL){
		char * current = calloc(65 , sizeof(char));
		strcpy(current , each_param);
		*(parsed_input + input_count) = current;
		input_count += 1;
		each_param = strtok(NULL , " ");
	}

	return input_count;
}


int find_exe_path(int path_count , char ** parsed_input, char** all_path){
	int exe_index = -1;
	for(int i = 0; i < path_count; i++){
		if(*(all_path+i) == NULL) break;

		char * temp = calloc(200 , sizeof(char));
		strcpy(temp , *(all_path + i));

		*(temp + strlen(temp)) = '/';

		for(int j = 0; j < strlen(*parsed_input) ; j++){
			*(temp + strlen(temp)) = *(*(parsed_input) + j);
		}
		struct stat buffer;
		int status = lstat(temp , &buffer);

		if(status == 0){
			exe_index = i;
			free(temp);
			break;
		}

		free(temp);

	}
	return exe_index;
}

void child_process_status(int* child_process_count){
	int temp = *child_process_count;
	for(int i = 0; i < temp; i++){
		int status;

		int pid = waitpid(-1 , &status, WNOHANG);

		if(pid == 0) continue;

		if (WIFSIGNALED(status) || WEXITSTATUS(status) != 0){
			printf("<background process terminated abnormally>\n");
			*child_process_count -= 1;
		}else{
			printf("<background process terminated with exit status %d>\n",WEXITSTATUS(status) );
			*child_process_count -=1;
		}
	}
}


int cd_command(int input_count , char ** parsed_input ){

	int status = -1;

	if(input_count == 1){
		status = chdir(getenv("HOME"));
	}

	else if( strcmp( *(parsed_input+1) , "/") == 0){
		status = chdir("/");
	}

	else{
		status = chdir( *(parsed_input + 1) );
	}

	return status;

}


int main()
{
	setvbuf( stdout, NULL, _IONBF, 0 );

	char** all_path = calloc(50 , sizeof(char*));
	int path_count = parse_path_funct(all_path);
	int child_process_count = 0;

	while(1){

		char* user_input = calloc(1025 , sizeof(char));


		child_process_status(&child_process_count);
		printf("$ ");
			
		//read input
		fgets(user_input , 1025 , stdin);
		*(user_input + strlen(user_input) - 1) = '\0';

		//break the loop
		if(strlen(user_input) == 0) {
			free(user_input);
			continue;
		}
		if(strcmp(user_input , "exit") == 0 ){
			printf("bye\n");
			free(user_input);
			break;
		}


		//parse the user input 
		char** parsed_input = calloc(64 , sizeof(char*));
		int input_count = parse_input_funct(parsed_input , user_input);

		if(input_count == 0){
			for(int i = 0; i < input_count; i++){
				if(*(parsed_input + i) != NULL) free(*(parsed_input + i));
			}
			free(parsed_input);
			free(user_input);

			continue;
		}

		//cd
		int cd = 0;
		if(strcmp(*parsed_input , "cd") == 0){

			cd = 1;
			int status = cd_command(input_count , parsed_input);
			if (status == -1){
				fprintf(stderr,"cd: %s: No such file or directory\n", *(parsed_input+1));
			}
		}
		


		//check if that path exist

		int exe_index = find_exe_path(path_count , parsed_input , all_path);
		

		//deal with backgroud running
		int background = 0;
		if(strcmp( *(parsed_input + input_count - 1), "&" ) == 0){
			background = 1;
			free(*(parsed_input + input_count - 1));
			*(parsed_input + input_count - 1) = '\0';
		}


		//exec that file

		if(exe_index != -1 && cd == 0){

			//set up the executable dir
			char * exe_dir = calloc(strlen(*(all_path + exe_index)) + strlen(*parsed_input) + 2, sizeof(char));
			strcpy(exe_dir , *(all_path + exe_index) );
			*(exe_dir + strlen(exe_dir)) = '/';

			for(int i = 0; i < strlen(*parsed_input) ; i ++){
				*(exe_dir + strlen(exe_dir)) = *(*parsed_input + i);
			}

			*(exe_dir + strlen(exe_dir)) = '\0';



			pid_t pid;
			pid = fork();

			if(pid == -1) {
				perror("fork() failed");
			}

			if(pid == 0){ //child

				execv(exe_dir , parsed_input);
				perror("Exec failed");

			}

			if(pid > 0){ //parent
				int status;
				if(background == 0)
					waitpid(pid , &status , 0);
				else{


					printf("<running background process \"%s\">\n", *parsed_input);
					child_process_count += 1;
					int child_pid = waitpid(pid ,&status ,WNOHANG );

					if(child_pid == -1){
						perror("waitpid() failed");
						free(user_input);
						continue;
					}


				}


			}

			free(exe_dir);

		}

		else if (cd == 0){
			fprintf(stderr,"%s: command not found\n", *parsed_input);
		}

		
		//free user input and parsed input each loop
		for(int i = 0; i < input_count; i++){
			if(*(parsed_input + i) != NULL) free(*(parsed_input + i));
		}
		free(parsed_input);
		free(user_input);

	}

	//free the path
	
	for(int i = 0; i < path_count; i++){
		if(*(all_path + i) != NULL) free(*(all_path + i));
	} 
	free(all_path);

	return 0;
}
