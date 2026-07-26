#include "../include/unimplemented.h"
#include <stdio.h>

// Creating a dynamic array

void *init_array(size_t array_size)
{
	void *array_p = malloc(array_size);

	if (array_p == NULL)
	{
		return NULL;
	}

	return array_p;
};

int array_len(int *array_header){
    return *array_header;
}

void array_append(int *array_header, int item){
    size_t offset = sizeof(*array_header) + *array_header;
    array_header[offset] = item;
}

int array_pop(){
    UNIMPLEMENTED;
}

int array_remove(){
    UNIMPLEMENTED;
}


int main()
{
	// Initialize Dynamic Array
	int capacity = 10;
	int array_length = 2;

    // Accounting for array header
	size_t array_size = sizeof(array_length) + (sizeof(int) * capacity);
	size_t array_size_char = sizeof(array_length) + (sizeof(char) * capacity);

    // Growth factor for increasing the array size when maxed
	const double growth_factor = 2;

	// Create dynamic array
	int *array_header = init_array(array_size);
	int *array_header_char = init_array(array_size_char);

	// Incase operating system denies like why would it do that
	if (array_header == NULL || array_header_char == NULL)
	{
		return 1;
	}

	// Let's create little meta data for our array that is array[0]
	// The first element in our array
	*array_header = array_length;

	// *array_header_char = array_length;
    // printf("%d\n", array_len(array_header));

    array_append(array_header, 2);

    for (int i=0; i < *array_header; ++i){
        printf("%d\n", array_header[i]);
    }

	// Let's Create initialize our offset
	// Keeping track of the array header
	// size_t current_offset = sizeof(*array_header) + (sizeof(int) * array_length);
	// size_t current_offset_char = sizeof(*array_header_char) + (sizeof(char) * array_length);

    (void) growth_factor;

    // printf("%zu\n", current_offset);
    // printf("%zu\n", current_offset_char);

	return 0;


    // assert(current_offset % *(array_header + 1) == 0);
    // int current_position = current_offset / *(array_header + 1);

    // printf("%zu\n", current_offset);
    // printf("%zu\n", current_offset_char);

	// // Dummy append to the array for fun I guess
	// for (int i = 0; i < 100; ++i)
	// {
	// 	// check if you can actually add
	// 	// Like the array can be filled up STOP THAT!
	// 	if ((sizeof(i) * array_length) >= array_size)
	// 	{
	//            // int new_capacity = (capacity * growth_factor) + 1;
	//            // int * bigger_array = init_array(array_size * new_capacity);
	//            // *bigger_array = *array_p;
	//            // for(int j = 0; j < capacity; ++j){
	//            //     bigger_array[]
	//            // }
	// 		return 1;
	// 	}
	// 	++array_length;
	// 	array_p[i + current_offset] = i;
	//
	//     printf("%d\n", *(array_p + i));
	// }
	//
	//    (void) growth_factor;
	//    (void) current_offset;
	// printf("%zu\n", array_size);
}
