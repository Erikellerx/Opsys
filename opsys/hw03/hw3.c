#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


extern int next_thread_id;
extern int max_squares;
extern char *** dead_end_boards;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int found = 0;
int dead_end_board_index = 0;
int parallel = 0;
pthread_t  threads[100000];
int least_square = 0;
int dead_end_board_size = 4;
int max = 0;


struct Point{
    int x;
    int y;
};

struct Board
{
    int row; 
    int col;
    int covered; 
    struct Point currentPoint; 
    int** data; 
    int id;
};

void create_thread(int validnum , struct Point* validPoints, struct Board * board, int* step, int id);



void print_result(int row , int col){
    for(int i = 0; i < dead_end_board_index; i++){

        printf("MAIN: >>");

        for(int j = 0; j < row; j ++){
            if(j != 0) printf("MAIN:   ");
            for(int k = 0; k < col; k++){
                printf("%c", dead_end_boards[i][j][k]);
            }
            if (j != row-1)printf("\n");
        }
        printf("<<\n");
    }
    

}

void copy_board(struct Board* old , struct Board* new){

    new->row = old->row;
    new->col = old->col;
    new->covered = old->covered;
    new->id = old->id;


    new->data = calloc(old->row, sizeof(int*));

    for(int i = 0; i < new->row ; i++){
        new->data[i] = calloc(new->col , sizeof(int));
        for(int j = 0; j < new->col; j++){
            new->data[i][j] = old->data[i][j];
        }
    }

}

int get_id(pthread_t t){
    for(int i = 0; i < next_thread_id; i++){
        if(threads[i] == t) return i;
    }
    return -1;
}

void remove_thread(int index){
    for(int i = index; i < 10000 - 1; i++) threads[i] = threads[i+1];
}


int is_valid_point(struct Point point, struct Board board){
    return point.x >= 0 && point.y >= 0 && point.x < board.row && point.y < board.col && board.data[point.x][point.y] == 0;
}

int count_valid_path(struct Point point, struct Board board, struct Point* validPoints){
    int num = 0;

    struct Point p1;
    p1.x = point.x + 1;
    p1.y = point.y + 2;
    if (is_valid_point(p1, board)){
        validPoints[num] = p1;
        num++;
    }

    struct Point p2;
    p2.x = point.x - 1;
    p2.y = point.y + 2;
    if (is_valid_point(p2, board)){
        validPoints[num] = p2;
        num++;
    }

    struct Point p3;
    p3.x = point.x - 2;
    p3.y = point.y + 1;
    if (is_valid_point(p3, board)){
        validPoints[num] = p3;
        num++;
    }

    struct Point p4;
    p4.x = point.x - 2;
    p4.y = point.y - 1;
    if (is_valid_point(p4, board)){
        validPoints[num] = p4;
        num++;
    }

    struct Point p5;
    p5.x = point.x - 1;
    p5.y = point.y - 2;
    if (is_valid_point(p5, board)){
        validPoints[num] = p5;
        num++;
    }

    struct Point p6;
    p6.x = point.x + 1;
    p6.y = point.y - 2;
    if (is_valid_point(p6, board)){
        validPoints[num] = p6;
        num++;
    }

    struct Point p7;
    p7.x = point.x + 2;
    p7.y = point.y - 1;
    if (is_valid_point(p7, board)){
        validPoints[num] = p7;
        num++;
    }
    
    struct Point p8;
    p8.x = point.x + 2;
    p8.y = point.y + 1;
    if (is_valid_point(p8, board)){
        validPoints[num] = p8;
        num++;
    }
   
    return num;
}

void updata_dead_end_boards(struct Board* board){
    struct Board b;
    copy_board(board, &b);
    if(least_square > b.covered) return;
    pthread_mutex_lock(&mutex);

    if(dead_end_board_size <= dead_end_board_index){
        dead_end_boards = realloc(dead_end_boards , dead_end_board_size * 2 * sizeof(char**));
        dead_end_board_size *= 2;
    }

    dead_end_boards[dead_end_board_index] = calloc(b.row, sizeof(char*));
    for(int i = 0; i < b.row; i++){
        dead_end_boards[dead_end_board_index][i] = calloc(b.col, sizeof(char));
        for(int j = 0; j < b.col; j++){
            if(b.data[i][j] == 0) dead_end_boards[dead_end_board_index][i][j] = '.';
            if(b.data[i][j] != 0) dead_end_boards[dead_end_board_index][i][j] = 'S';
        }
    }
   

    dead_end_board_index += 1;

    pthread_mutex_unlock(&mutex);
}

void updata_board(struct Board * board , struct Point * valid_points){

    board -> data[valid_points->x][valid_points -> y] = board -> covered + 1;
    board->covered += 1;
    board->currentPoint = *valid_points;

    pthread_mutex_lock(&mutex);
    if(max_squares < board -> covered) max_squares = board -> covered;
    pthread_mutex_unlock(&mutex);

}


void  run_no_thread(struct Board * board , int * step){
    struct Point * valid_points = calloc(8, sizeof(struct Point));
    int validnum = count_valid_path(board -> currentPoint , *board , valid_points);

    if(validnum > 1){
        printf("THREAD %d: %d possible moves after move #%d; creating %d child threads...\n",board->id,validnum, board->covered ,validnum);
        create_thread(validnum , valid_points , board, step, board->id);
    }else if(validnum > 0){
        updata_board(board , valid_points);

        pthread_mutex_lock(&mutex);

        *step = board->covered;
;
        pthread_mutex_unlock(&mutex);
        run_no_thread(board, step);

    }else{
        if(board -> covered < max){
            updata_dead_end_boards(board);
            printf("THREAD %d: Dead end at move #%d\n", board->id ,board ->covered );
        }else{
            printf("THREAD %d: Sonny found a full knight's tour!\n", board->id);
            pthread_mutex_lock(&mutex);
            found += 1;
            pthread_mutex_unlock(&mutex);
        }

    }
}


void * run(void * arg){
    struct Board * board = (struct Board*) arg;


    int* step = calloc(1 , sizeof(int));
    *step = board -> covered;

    struct Point * valid_points = calloc(8 , sizeof(struct Point));
    int validnum = count_valid_path(board->currentPoint , *board , valid_points );

    if(validnum > 1){
        printf("THREAD %d: %d possible moves after move #%d; creating %d child threads...\n",board -> id,validnum, board->covered ,validnum);
        create_thread(validnum , valid_points , board, step, board -> id);

    }else if(validnum == 1){
        updata_board(board , valid_points);

        pthread_mutex_lock(&mutex);
        *step = board->covered;
        pthread_mutex_unlock(&mutex);
        
        run_no_thread(board, step);

    }else{
        if(board -> covered < max){
            updata_dead_end_boards(board);
            printf("THREAD %d: Dead end at move #%d\n", board-> id,board ->covered );
        }else{
            printf("THREAD %d: Sonny found a full knight's tour!\n", board->id);
            pthread_mutex_lock(&mutex);
            found += 1;
            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_exit(step);
}

void create_thread(int validnum , struct Point* validPoints, struct Board * board, int * step, int id){ 
    int rc;
    pthread_t* pid = malloc(validnum * sizeof(pthread_t));

    for(int i = 0; i < validnum; i++){
        struct Point p = validPoints[i];
        struct Board *temp = calloc(1 , sizeof(struct Board));

        copy_board(board,temp);

        pthread_mutex_lock(&mutex);
        //update
        temp->data[p.x][p.y] = temp -> covered + 1;
        temp->covered += 1;
        temp->currentPoint = validPoints[i];
        temp->id = next_thread_id;
        //printf("temp->id: %d\n", temp->id);
      
        next_thread_id ++;
        if(max_squares < board -> covered) max_squares = board -> covered;
        pthread_mutex_unlock(&mutex);

        rc = pthread_create(&pid[i] , NULL , run , (void*) temp);

        pthread_mutex_lock(&mutex);
        threads[next_thread_id-1] = pid[i];
        pthread_mutex_unlock(&mutex);

        if(rc != 0){
            fprintf(stderr, "ERROR: Could not create thread\n");
        }

        if(parallel == 1){
            int * x;
            rc = pthread_join(pid[i] , (void **) &x);
            pthread_mutex_lock(&mutex);

            if(*x > *step)*step = *x;
            pthread_mutex_unlock(&mutex);

            if(rc != 0) fprintf(stderr, "ERROR: No thread to join!\n");
            else {

                if(id == 0)
                printf("MAIN: Thread %d joined (returned %d)\n", temp -> id, *x);
                else
                printf("THREAD %d: Thread %d joined (returned %d)\n", id , temp->id , *x);

            }



        }

    }

    if(parallel == 0){
        for(int i = 0; i < validnum; i++){
            int *x;
            rc = pthread_join(pid[i] , (void**) &x);

            

            if(rc != 0){
                fprintf(stderr, "ERROR: No thread to join!\n");
            }else{
                printf("MAIN: Thread %d joined (returned %d)\n",  get_id(pid[i]), *x);
            }

        }
    }

}


int simulate(int argc, char* argv[]){

    setvbuf( stdout, NULL, _IONBF, 0 );
    ///TODO: Error checking

    #ifdef NO_PARALLEL
    parallel = 1;
    #endif

    next_thread_id = 1;

    if(argc != 6){
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> <r> <c> <x>\n");
        return EXIT_FAILURE;
    }


    int row = atoi(argv[1]);
    int col = atoi(argv[2]);

    int start_row = atoi(argv[3]);
    int start_col = atoi(argv[4]);

    least_square = atoi(argv[5]);

    if(least_square > row * col){
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> <r> <c> <x>\n");
        //next_thread_id += 1;
        return EXIT_FAILURE;
    }



    int** board = calloc(row, sizeof(int*));
    for(int i = 0; i < row; i++){
        *(board + i) = calloc(col, sizeof(int));
    }
    board[start_row][start_col] = 1;

    struct Board init_board;
    init_board.row = row;
    init_board.col = col;
    init_board.data = board;
    init_board.covered = 1;
    max = row * col;
    init_board.id = 0;

    struct Point init_point;
    init_point.x = start_row;
    init_point.y = start_col;

    init_board.currentPoint = init_point;

    printf("MAIN: Solving Sonny's knight's tour problem for a %dx%d board\n", row, col);

    printf("MAIN: Sonny starts at row %d and column %d (move #1)\n", start_row, start_col);


    struct Point* valid_points = malloc(8 * sizeof(struct Point));
    int validnum = count_valid_path(init_point , init_board, valid_points);
    int * step = calloc(1 , sizeof(int));
    if(validnum > 1){
        
        printf("MAIN: %d possible moves after move #%d; creating %d child threads...\n", validnum , init_board.covered ,validnum);
        create_thread(validnum , valid_points , &init_board , step, init_board.id);
    }else if(validnum == 1){
        updata_board(&init_board , valid_points);

        pthread_mutex_lock(&mutex);
        *step = init_board.covered;
        pthread_mutex_unlock(&mutex);
        run_no_thread(&init_board, step);

    }else{
        pthread_mutex_lock(&mutex);
        max_squares += 1;
        pthread_mutex_unlock(&mutex);
        if(init_board.covered < max){
            updata_dead_end_boards(&init_board);
            printf("MAIN: Dead end at move #%d\n",init_board.covered );
        }else{
            printf("MAIN: Sonny found a full knight's tour!\n");
        }
    }

    if(found != 0){
        printf("MAIN: All threads joined; found %d possible ways to achieve a full knight's tour", found);
    }else{

        if(max_squares == 1)printf("MAIN: All threads joined; best solution(s) visited %d square out of %d\n", max_squares,row * col);
        else printf("MAIN: All threads joined; best solution(s) visited %d squares out of %d\n", max_squares,row * col);

        if(least_square == 1 && dead_end_board_index == 1) printf("MAIN: Dead end board covering at least %d square:\n" , least_square);
        else if(least_square == 1) printf("MAIN: Dead end boards covering at least %d square:\n" , least_square);
        else if(dead_end_board_index == 1) printf("MAIN: Dead end board covering at least %d squares:\n" , least_square);
        else printf("MAIN: Dead end boards covering at least %d squares:\n" , least_square);
        if(dead_end_board_index > 0){

            print_result(row, col);
        }
    }

    
    //next_thread_id += 1;


    return EXIT_SUCCESS;

}