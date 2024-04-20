%%writefile cuda_example.cu
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>

#define CUDA_CHECK(call) \
do { \
    cudaError_t cudaErr = (call); \
    if (cudaErr != cudaSuccess) { \
        fprintf(stderr, "CUDA Error - %s:%d: '%s'\n", __FILE__, __LINE__, cudaGetErrorString(cudaErr)); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>

#define CUDA_CHECK(call) \
do { \
    cudaError_t cudaErr = (call); \
    if (cudaErr != cudaSuccess) { \
        fprintf(stderr, "CUDA Error - %s:%d: '%s'\n", __FILE__, __LINE__, cudaGetErrorString(cudaErr)); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

__global__ void pigeonhole_sort_kernel(int* arr, int n, int min_val, int range_size) {
    extern __shared__ int local_pigeonholes[];

    int tid = threadIdx.x + blockIdx.x * blockDim.x;
    int stride = blockDim.x * gridDim.x;

    // Initialize local pigeonholes
    for (int i = threadIdx.x; i < range_size; i += blockDim.x) {
        local_pigeonholes[i] = 0;
    }
    __syncthreads();

    // Count occurrences in local pigeonholes
    while (tid < n) {
        atomicAdd(&local_pigeonholes[arr[tid] - min_val], 1);
        tid += stride;
    }
    __syncthreads();

    // Perform parallel prefix sum (scan) to calculate starting indices
    for (int i = 1; i < range_size; i <<= 1) {
        if (threadIdx.x >= i) {
            local_pigeonholes[threadIdx.x] += local_pigeonholes[threadIdx.x - i];
        }
        __syncthreads();
    }

    // Write sorted values to shared memory in correct order
    tid = threadIdx.x + blockIdx.x * blockDim.x;
    while (tid < n) {
        int index = atomicSub(&local_pigeonholes[arr[tid] - min_val], 1) - 1;
        arr[index] = arr[tid];
        tid += stride;
    }
}


int main() {
    cudaEvent_t sort_start, sort_end;
    float sort_time;

    int n;
    int* arr = NULL;

    CUDA_CHECK(cudaEventCreate(&sort_start));
    CUDA_CHECK(cudaEventCreate(&sort_end));
    int capacity = 100000;
    arr = (int*)malloc(capacity * sizeof(int));

    FILE* file = fopen("/content/drive/MyDrive/T10I4D100K.dat.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open the file for reading.\n");
        free(arr);
        return 1;
    }

    printf("Numbers read from the file: ");
    int count = 0;

    while (fscanf(file, "%d", &arr[count]) != EOF) {
        count++;

        if (count >= capacity) {
            // printf("Too many integers in the file. Increase the array size if necessary.\n");
            break;
        }
    }

    fclose(file);

    n = count; // Set the total number of elements.

    // Copy data to GPU
    int* d_arr;
    CUDA_CHECK(cudaMalloc((void**)&d_arr, n * sizeof(int)));
    CUDA_CHECK(cudaMemcpy(d_arr, arr, n * sizeof(int), cudaMemcpyHostToDevice));

    int range_size = 100000;  // Adjust this value based on your data range
    int min_val = 0;          // Adjust this value based on your data range

    // Calculate block and grid dimensions
    int block_size = 256;
    int grid_size = (n + block_size - 1) / block_size;
    CUDA_CHECK(cudaEventRecord(sort_start));
    pigeonhole_sort_kernel<<<grid_size, block_size, range_size * sizeof(int)>>>(d_arr, n, min_val, range_size);
      CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaEventRecord(sort_end));
    CUDA_CHECK(cudaEventSynchronize(sort_end));
    CUDA_CHECK(cudaEventElapsedTime(&sort_time, sort_start, sort_end));


    printf("\nSorting function execution time: %f seconds\n", sort_time / 1000.0);

    // Copy sorted data back to the host
    CUDA_CHECK(cudaMemcpy(arr, d_arr, n * sizeof(int), cudaMemcpyDeviceToHost));
    printf("\nSorted Data: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    CUDA_CHECK(cudaDeviceSynchronize());

    // You can now use the arr for further processing or output.
    // Dont forget to free any dynamically allocated memory.
    free(arr);
    CUDA_CHECK(cudaFree(d_arr));

    return 0;
}

