#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024
#define MAX_TOKEN_LENGTH 128

// Shared data structures
char shared_queue[MAX_LINES][MAX_LINE_LENGTH];
int queue_front = 0;
int queue_rear = 0;
int queue_size = 0;

// Mutex for queue access
omp_lock_t queue_lock;

// Producer function
void producer(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file: %s\n", filename);
        return;
    }
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        omp_set_lock(&queue_lock);
        if (queue_size < MAX_LINES) {
            strcpy(shared_queue[queue_rear], line);
            queue_rear = (queue_rear + 1) % MAX_LINES;
            queue_size++;
        }
        omp_unset_lock(&queue_lock);
    }
    fclose(file);
}

// Consumer function
void consumer() {
    char token[MAX_TOKEN_LENGTH];
    while (1) {
        omp_set_lock(&queue_lock);
        if (queue_size > 0) {
            char* line = shared_queue[queue_front];
            queue_front = (queue_front + 1) % MAX_LINES;
            queue_size--;
            omp_unset_lock(&queue_lock);
            char* token = strtok(line, " \t\n");
            while (token != NULL) {
                printf("Consumer %d: %s\n", omp_get_thread_num(), token);
                token = strtok(NULL, " \t\n");
            }
        } else {
            omp_unset_lock(&queue_lock);
            break; // No more lines to process
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <file1> <file2> ... <fileN> <num_consumers>\n", argv[0]);
        return 1;
    }
    int num_consumers = atoi(argv[argc - 1]);
    if (num_consumers <= 0) {
        printf("Invalid number of consumers.\n");
        return 1;
    }
    omp_init_lock(&queue_lock);
    
    // Create producer threads
    #pragma omp parallel for
    for (int i = 1; i < argc - 1; i++) {
        producer(argv[i]);
    }
    
    // Create consumer threads
    #pragma omp parallel num_threads(num_consumers)
    {
        consumer();
    }
    
    omp_destroy_lock(&queue_lock);
    return 0;
}