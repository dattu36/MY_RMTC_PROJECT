#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void pigeonhole_sort(int arr[], int n, int min_val, int range_size, int* sorted_arr) {
    int pigeonholes[range_size];
    for (int i = 0; i < range_size; i++) {
        pigeonholes[i] = 0;
    }

    for (int i = 0; i < n; i++) {
        pigeonholes[arr[i] - min_val]++;
    }

    int index = 0;
    for (int i = 0; i < range_size; i++) {
        while (pigeonholes[i] > 0) {
            sorted_arr[index++] = i + min_val;
            pigeonholes[i]--;
        }
    }
}

double getCurrentTime() {
    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    return (double)currentTime.tv_sec * 1000.0 + (double)currentTime.tv_nsec / 1000000.0;
}

int main() {
    int n;
 
    int capacity = 10;  // Initial capacity for the array
    int *arr = malloc(capacity * sizeof(int));

   double startTime = getCurrentTime();
    // Open the file for reading
    FILE *file = fopen("T10I4D100K.dat.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open the file for reading.\n");
        free(arr);  // Free allocated memory before exiting
        return 1;
    }

    printf("Numbers read from the file: ");
    int count = 0;

    // Read each number from the file
    while (fscanf(file, "%d", &arr[count]) != EOF) {
      //  printf("%d ", arr[count]);
        count++;

        // Resize the array if needed
        if (count >= capacity) {
            capacity *= 2;
            arr = realloc(arr, capacity * sizeof(int));
        }
    }

    // Close the file
    fclose(file);
     double endTime = getCurrentTime();
       double totalTime = endTime - startTime;
    printf("Time taken by the loading: %.5f milliseconds\n", totalTime);

    startTime = getCurrentTime();
    int min_val = arr[0];
    int max_val = arr[0];
    for (int i = 1; i < count; i++) {
        if (arr[i] < min_val) {
            min_val = arr[i];
        }
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }

    int range_size = max_val - min_val + 1;
    int *sorted_arr = malloc(count * sizeof(int));
    
    pigeonhole_sort(arr, count, min_val, range_size, sorted_arr);
         endTime = getCurrentTime();

    //printf("\nSorted array: ");
   // for (int i = 0; i < count; i++) {
     //   printf("%d ", sorted_arr[i]);
   // }
   // printf("\n");


     totalTime = endTime - startTime;
    printf("Time taken by the sorting: %.5f milliseconds\n", totalTime);

    // Free allocated memory
    free(arr);
    free(sorted_arr);

    return 0;
}

