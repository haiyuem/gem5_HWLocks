#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>
#include <stdalign.h>

#define CACHELINE_SIZE 64

void load_data(char * file_name, long long input_data_length, unsigned char ** parsed_array, int number_buckets)
{
    FILE * fr;
    unsigned char * local_parsed_array = malloc(sizeof(unsigned char)*input_data_length);
    char line[80];
    assert(local_parsed_array != 0);
    *parsed_array = local_parsed_array;
    long long counter;
    long data;
    fr = fopen (file_name, "rt");
    if(fr == 0)
    {
        printf("unable to open file %s\n", file_name);
        exit(2);
    }
    assert(fr != 0);
    for(counter = 0; counter < input_data_length; counter++)
    {
        assert(fgets(line, 80, fr) != 0);
        assert(sscanf(line, "%ld", &data) != 0);
        assert(data >= 0 && data < number_buckets);
        local_parsed_array[counter] = (unsigned char) data;
    }
    fclose(fr);
}

// Cacheline-aligned wrappers for long long, lock.
typedef struct _aligned_lock_t{
    alignas(CACHELINE_SIZE) pthread_mutex_t lock;
} aligned_lock_t;

typedef struct _aligned_long_long_t{
    alignas(CACHELINE_SIZE) long long n;
} aligned_long_long_t;

// Arguments to be passed to each thread as a void pointer.
typedef struct _thread_args_t{
    aligned_long_long_t* local_output_data;
    aligned_lock_t* mut;
    unsigned char* input_data;
    long long input_data_length;
} thread_args_t;

// Function for each thread to run.
void* thread_fct(void* ptr) {
    thread_args_t* args = (thread_args_t*) ptr;
    long long counter;
    for (counter = 0; counter < args->input_data_length; counter++) {
        pthread_mutex_lock(&args->mut[args->input_data[counter]].lock);
        args->local_output_data[args->input_data[counter]].n++;
        pthread_mutex_unlock(&args->mut[args->input_data[counter]].lock);
    }
    pthread_exit(NULL);
}

void compute_histogram_d(long long ** output_data, unsigned char * input_data, long long input_data_length, int number_buckets, int number_threads)
{
    long long * local_output_data = malloc(sizeof(long long) * number_buckets);
    assert(local_output_data != 0);
    *output_data = local_output_data;
    int counter_local;
    for(counter_local = 0; counter_local < number_buckets; counter_local++)
    {
        local_output_data[counter_local] = 0;
    }

    // Tranche input_data evenly across the number of threads.
    pthread_t* thread = malloc(sizeof(pthread_t) * number_threads);
    thread_args_t* thread_args = malloc(sizeof(thread_args_t) * number_threads);
    // mutex for each bucket
    aligned_lock_t* mut = malloc(sizeof(aligned_lock_t) * number_buckets); 
    for (counter_local = 0; counter_local < number_buckets; counter_local++) {
        pthread_mutex_init(&mut[counter_local].lock, NULL);
        printf("lock address: %p\n", (void *)&mut[counter_local]);
    }
    // array of aligned long long wrappers
    aligned_long_long_t* aligned_output = malloc(sizeof(aligned_long_long_t) * number_buckets);

    long long step_size = input_data_length / (number_threads);
    for (int i = 0; i < number_threads; i++) {
        // Assign arguments for the thread.
        thread_args[i].local_output_data = aligned_output;
        thread_args[i].mut = mut;
        thread_args[i].input_data = input_data + step_size*i;
        // If we're the last thread, just take the remainder of the input.
        thread_args[i].input_data_length = 
            (i == number_threads-1) ? (input_data_length - step_size*i) : step_size;
        pthread_create(&thread[i], NULL, thread_fct, &thread_args[i]);
    }

    // Wait on all threads.
    for (int i = 0; i < number_threads; i++) {
        pthread_join(thread[i], NULL);
    }

    // Write aligned long longs to local_output_data.
    for (int i = 0; i < number_buckets; i++) {
        local_output_data[i] = aligned_output[i].n;
    }

    free(thread);
    free(thread_args);
    free(mut);
    free(aligned_output);
}

void print_histogram(int number_buckets, long long * histogram)
{
    int counter;
    for(counter = 0; counter < number_buckets; counter++)
    {
        printf("%d: %ld\n", counter, histogram[counter]);
    }
}

void usage(int argc, char * argv[])
{
    printf("Usage: %s INPUT_FILE INPUT_LENGTH MAX_INPUT_NUMBER NUMBER_THREADS\n", argv[0]);
    exit(2);
}

int main(int argc, char * argv[], char**envp)
{
    unsigned char * data_array;
    long long * histogram_array;
    struct timespec res, t1, t2;
    if(argc != 5)
    {
        usage(argc, argv);
    }
    // assert (atoi(argv[4]) == 1);
    // FIXME, should have better error checking on these atoi's as atoi has
    // bad error handling
    load_data(argv[1], atoi(argv[2]), &data_array, atoi(argv[3])+1);
    // Get the starting time
    // clock_gettime(CLOCK_REALTIME, &t1);
    compute_histogram_d(&histogram_array, data_array, atoi(argv[2]), atoi(argv[3])+1, atoi(argv[4]));
    // Get the ending time
    // clock_gettime(CLOCK_REALTIME, &t2);
    // print_histogram(atoi(argv[3])+1, histogram_array);
    // Print out the time taken
    // printf("### test took %1.31f seconds\n", (t2.tv_sec - t1.tv_sec)  + (float) (t2.tv_nsec - t1.tv_nsec) / 1000000000.0);
}
