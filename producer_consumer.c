#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define N 5 //Queue size of 5

char buffer [N];
int head = 0;
int tail = 0;
int count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void *producer(void *v)
{
    int c;

    FILE *fp = fopen("message.txt", "r"); //ope file w/ read permissions
    if (fp == NULL)
    {
        printf ("File could not be opened. Exiting"); //check if error opening file. exit if so 
        exit(-1);
    }

    
    while( (c = fgetc(fp)) != EOF) //loop while not end of file
    {
        pthread_mutex_lock(&mutex); //get mutex 
        while(count == N) //check if buffer is full
            pthread_cond_wait(&not_full, &mutex); //sleep

        buffer [tail] = (char)c; //add character to buffer
        tail = (tail + 1) % N; //advance pointers + account for wraparound on tail
        count++;

        printf("Producer read in %c \n", (char)c);

        pthread_mutex_unlock(&mutex); //release mutex
        pthread_cond_signal(&not_empty); //update not empty
    }
    //end of file, close file pointer
    fclose(fp);
    //get lock , send /0 to indicate to consumer end of message.

    pthread_mutex_lock(&mutex);

    while(count == N) //check if buffer is full
            pthread_cond_wait(&not_full, &mutex);

        buffer [tail] = '\0'; //add EOF to signal consumer
        tail = (tail + 1) % N; //advance pointers + account for wraparound on tail
        count++;

        printf("Producer reached end of message \n");

        pthread_mutex_unlock(&mutex); //release mutex
        pthread_cond_signal(&not_empty); //update not empty


    pthread_exit(NULL);
}

void *consumer(void *v)
{
    
    while(1)
    {
        pthread_mutex_lock(&mutex); //get mutex
        char c;

        while(count == 0)
            pthread_cond_wait(&not_empty, &mutex);

        c = buffer [head]; //read in character from queue
        head = (head + 1) % N; //update pointer + handle wraparound
        count--; //decrement count

        pthread_cond_signal(&not_full); //update not full
        pthread_mutex_unlock(&mutex); //release 

        if(c == '\0') //end of message
        {
            printf("Consumer reached end of message \n");
            break;
        } 
        printf("Consumer read from queue: %c \n", c);
    }
    
    pthread_exit(NULL);
}



int main()
{
    pthread_t consumerThread, producerThread; //init threads w/ respective functions
    pthread_create(&consumerThread, NULL, consumer, NULL);
    pthread_create(&producerThread, NULL, producer, NULL);

    pthread_join(producerThread, NULL);
    pthread_join(consumerThread, NULL);

    pthread_mutex_destroy(&mutex); //close threads and conditions
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);


    return 0;
}