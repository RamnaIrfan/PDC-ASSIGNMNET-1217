#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 500000    
#define CHUNK_SIZE 100 

void merge(int arr[], int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int* L = (int*)malloc(n1 * sizeof(int));
    int* R = (int*)malloc(n2 * sizeof(int));

    for (i = 0; i < n1; i++) L[i] = arr[left + i];
    for (j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

void parallelMergeSort(int arr[], int left, int right, int depth, int max_threads) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        if (depth < max_threads) {
            #pragma omp parallel sections
            {
                #pragma omp section
                parallelMergeSort(arr, left, mid, depth + 1, max_threads);

                #pragma omp section
                parallelMergeSort(arr, mid + 1, right, depth + 1, max_threads);
            }
        } else {
            parallelMergeSort(arr, left, mid, depth + 1, max_threads);
            parallelMergeSort(arr, mid + 1, right, depth + 1, max_threads);
        }

        merge(arr, left, mid, right);
    }
}

void runExperiment(const char* scheduleType, int scheduleMode, int num_threads) {
    double total_time = 0.0;
    printf("\nExecuting with %s scheduling using %d threads:\n", scheduleType, num_threads);

    omp_set_num_threads(num_threads); // Set the number of threads

    for (int run = 1; run <= 10; run++) {
        int* arr = (int*)malloc(SIZE * sizeof(int));
        if (!arr) {
            printf("Memory allocation failed!\n");
            return;
        }

        // Fill array with random values
        #pragma omp parallel for schedule(static, CHUNK_SIZE)
        for (int i = 0; i < SIZE; i++) {
            arr[i] = rand() % 100000;
        }

        double start = omp_get_wtime();
        parallelMergeSort(arr, 0, SIZE - 1, 0, num_threads);
        double end = omp_get_wtime();

        double time_taken = end - start;
        total_time += time_taken;
        free(arr);
    }
    printf("\nAverage %s Execution Time with %d threads: %f seconds\n", scheduleType, num_threads, total_time / 10);
}

int main() {
    int thread_counts[] = {2, 4, 8};

    for (int i = 0; i < 3; i++) {
        int num_threads = thread_counts[i];

        // Run Static Scheduling first
        runExperiment("Static", 0, num_threads);

        // Run Dynamic Scheduling after Static for the same thread count
        runExperiment("Dynamic", 1, num_threads);
    }

    return 0;
}
