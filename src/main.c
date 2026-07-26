#include "../include/unimplemented.h"
#include <stdio.h>

// Creating a dynamic array

// int * init_array (){
// 	int *array_p = malloc(sizeof(int) * CAPACITY);
//
//     if (array_p == NULL){
//         return NULL;
//     }
//
//     *array_p = LENGTH;
//
//     return array_p;
// };
//
// int array_insert(int *array_p, int item){
//    if ((sizeof(*array_p) * count) >= array_p_size) {
//        return 1;
//    }
// }

int main()
{
    // Initialize Dynamic Array
    int capacity = 1;
    int count = 0;
    size_t array_size = sizeof(int) * capacity;

    // Create dynamic array
    int *array_p = malloc(array_size);

    // Incase operating system denies like why would it do that
    if (array_p == NULL){
        return 1;
    }

    // Let's create little meta data for our array that is array[0]
    // The first element in our array
    *array_p = count;

    // Dummy append to the array for fun I guess
    for (int i = 0; i < 100; ++i){
        // check if you can actually add
        // Like the array can be filled up STOP THAT!
        if((sizeof(i) * count) >= array_size){
            return 1;
        }
        ++count;
        array_p[i] = i;
    }

    printf("%zu\n", array_size);
	return 0;
}
