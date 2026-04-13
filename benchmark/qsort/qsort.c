#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

int partition (int arr[], int low, int high)
{
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j <= high - 1; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return i + 1;
}

void quickSort(int arr[], int low, int high)
{
    if (low < high) {
        int i = partition(arr, low, high);
        quickSort(arr, low, i - 1);
        quickSort(arr, i + 1, high);
    }
}

// Test harness
// Function to print an array
void printArray(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

// Function to run a test case
void runTest(int arr[], int size, const char* testName) {
    printf("=== %s ===\n", testName);
    printf("Before sorting: ");
    printArray(arr, size);

    quickSort(arr, 0, size - 1);
    
    printf("After sorting: ");
    printArray(arr, size);
    
    printf("\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <test number> [value1 value2 value3 ...]\n", argv[0]);
        printf("Example: %s 1\n", argv[0]);
        printf("Example: %s 8 10 7 8 9 1 5\n", argv[0]);
        return 1;
    }
    
    int testNumber = atoi(argv[1]);
    
    // If user-specified array is provided
    if (argc > 2) {
        int size = argc - 2;
        int* arr = (int*)malloc(size * sizeof(int));
        
        for (int i = 0; i < size; i++) {
            arr[i] = atoi(argv[i + 2]);
        }
        
        char testName[100];
        sprintf(testName, "Test %d", testNumber);
        runTest(arr, size, testName);
        
        free(arr);
        return 0;
    }
    
    return 0;
}