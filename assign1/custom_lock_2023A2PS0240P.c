#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define Total_Threads 50

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_read = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_write = PTHREAD_COND_INITIALIZER;

int active_readers = 0;
int waiting_readers = 0;
int active_writer = 0;
int waiting_writers = 0;

int counter = 0;

void pthread_lab1_read_lock(int tid);
void pthread_lab1_read_unlock(int tid);
void pthread_lab1_write_lock(int tid);
void pthread_lab1_write_unlock(int tid);


void pthread_lab1_read_lock(int tid){
    pthread_mutex_lock(&lock);
    waiting_readers++;

    while(active_writer || active_readers >= 2){
        pthread_cond_wait(&can_read, &lock);
    }

    waiting_readers--;
    active_readers++;

    printf("Reader %d | Active readers %d | Counter %d\n",
        tid, active_readers, counter
    );

    pthread_mutex_unlock(&lock);
}

void pthread_lab1_read_unlock(int tid){
    pthread_mutex_lock(&lock);

    active_readers--;

    printf("Reader %d | Active Readers = %d\n", tid, active_readers);

    if(active_readers == 0){
        pthread_cond_signal(&can_write);
    }

    pthread_mutex_unlock(&lock);
}


void pthread_lab1_write_lock(int tid){
    pthread_mutex_lock(&lock);
    waiting_writers++;

    while(active_writer || active_readers > 0){
        pthread_cond_wait(&can_write, &lock);
    }

    waiting_writers--;
    active_writer = 1;

    printf("Writer Entering %d | counter before %d \n", tid, counter);

    pthread_mutex_unlock(&lock);
}


void pthread_lab1_write_unlock(int tid){
    pthread_mutex_lock(&lock);

    active_writer = 0;

    printf("Writer Exiting %d | counter after %d \n", tid, counter);

    if(waiting_readers > 0){
        pthread_cond_broadcast(&can_read);
    }else{
        pthread_cond_signal(&can_write);
    }

    pthread_mutex_unlock(&lock);
}


void* thread_func(void* arg){
    int tid = *(int*)arg;

    if(tid % 2 == 0){
        pthread_lab1_read_lock(tid);
        pthread_lab1_read_unlock(tid);
    }else{
        pthread_lab1_write_lock(tid);
        counter++;
        pthread_lab1_write_unlock(tid);
    }

    return NULL;
}

int main(){
    pthread_t threads[Total_Threads];
    int ids[Total_Threads];

    printf("Creating Threads %d", Total_Threads);

    for(int i = 0; i < Total_Threads; i++){
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &ids[i]);
        printf("Main : thread %d created (%s)\n", i, (i % 2 == 0) ? "Reader" : "writer");
    }

    for(int i = 0; i < Total_Threads; i++){
        pthread_join(threads[i], NULL);
    }

    printf("Final Counter Value %d\n", counter);
    return 0;
}