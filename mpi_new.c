#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <time.h>

void pigeonhole_sort(int local_arr[], int local_n, int min_val, int range_size, int* sorted_arr) {
    int* local_pigeonholes = (int*)malloc(range_size * sizeof(int));
    for (int i = 0; i < range_size; i++) {
        local_pigeonholes[i] = 0;
    }

    for (int i = 0; i < local_n; i++) {
        local_pigeonholes[local_arr[i] - min_val]++;
    }

    int index = 0;
    for (int i = 0; i < range_size; i++) {
        while (local_pigeonholes[i] > 0) {
            sorted_arr[index++] = i + min_val;
            local_pigeonholes[i]--;
        }	
    }

    free(local_pigeonholes);
}

void merge(int arr1[], int arr2[], int size1, int size2, int merged_arr[]) {
    int i = 0, j = 0, k = 0;
    while (i < size1 && j < size2) {
        if (arr1[i] < arr2[j]) {
            merged_arr[k++] = arr1[i++];
        } else {
            merged_arr[k++] = arr2[j++];
        }
    }
    while (i < size1) {
        merged_arr[k++] = arr1[i++];
    }
    while (j < size2) {
        merged_arr[k++] = arr2[j++];
    }
}

int main(int argc, char** argv) {
    double load_start_time, load_end_time;
    double sort_start_time, sort_end_time;
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int n;
    int* arr = NULL;

    if (world_rank == 0) {
        load_start_time = MPI_Wtime();
        int capacity = 10;
        arr = (int*)malloc(capacity * sizeof(int));

        FILE* file = fopen("T10I4D100K.dat.txt", "r");
        if (file == NULL) {
            fprintf(stderr, "Could not open the file for reading.\n");
            free(arr);
            MPI_Finalize();
            return 1;
        }

        printf("Numbers read from the file: ");
        int count = 0;

        while (fscanf(file, "%d", &arr[count]) != EOF) {
         //   printf("%d ", arr[count]);
            count++;

            if (count >= capacity) {
                capacity *= 2;
                arr = (int*)realloc(arr, capacity * sizeof(int));
            }
        }

        fclose(file);

        n = count; // Set the total number of elements.
        load_end_time = MPI_Wtime();
        printf("\nData loading time: %f seconds\n", load_end_time - load_start_time);

       
    }
 sort_start_time = MPI_Wtime();
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int local_n = n / world_size;
    int local_arr[local_n];

    // Scatter data to all processes
    MPI_Scatter(arr, local_n, MPI_INT, local_arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform pigeonhole sort in parallel
    int range_size = 100000;  // Adjust this value based on your data range
    int min_val = 0;         // Adjust this value based on your data range
    int sorted_arr[local_n];
    pigeonhole_sort(local_arr, local_n, min_val, range_size, sorted_arr);

    // Gather the sorted subarrays
    int* sorted_data = NULL;
    if (world_rank == 0) {
        sorted_data = (int*)malloc(n * sizeof(int));
    }

    MPI_Gather(sorted_arr, local_n, MPI_INT, sorted_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);
     sort_end_time = MPI_Wtime();
    // Merge the sorted subarrays
    if (world_rank == 0) {
        int* final_sorted_arr = (int*)malloc(n * sizeof(int));
        int* subarray_sizes = (int*)malloc(world_size * sizeof(int));

        // Compute the size of each subarray
        for (int i = 0; i < world_size; i++) {
            subarray_sizes[i] = n / world_size;
        }

        // Adjust the size for the last process if necessary
        subarray_sizes[world_size - 1] += n % world_size;

        // Merge the subarrays using the merge function
        int offset = 0;
        for (int i = 0; i < world_size; i++) {
            merge(final_sorted_arr, sorted_data + offset, subarray_sizes[i], n / world_size, final_sorted_arr);
            offset += subarray_sizes[i];
        }

        // Calculate and print the sorting function execution time
   
        printf("\nSorting function execution time: %f seconds\n", sort_end_time - sort_start_time);

        // You can now use the final_sorted_arr for further processing or output.
        // Don't forget to free any dynamically allocated memory.
        free(final_sorted_arr);
        free(subarray_sizes);
    }

    // Cleanup and finalize MPI
    if (world_rank == 0) {
        free(sorted_data);
    }

    free(arr);
    MPI_Finalize();

    return 0;
}

