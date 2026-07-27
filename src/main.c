#include "../include/unimplemented.h"
#include <stdio.h>

#define GROWTH_FACTOR 2

struct array
{
	int count;
	int capacity;
};

struct array *init_array()
{
	struct array *arr = malloc(sizeof(*arr) + sizeof(int));

	if (arr == NULL)
	{
		return NULL;
	}

	arr->count = 0;
	arr->capacity = 1;

	return arr;
}

int array_len(struct array *arr)
{
	return arr->count;
}

int array_get(struct array *arr, int index)
{
	const int *data = (const int *)(arr + 1);
	return data[index];
}

int array_append(struct array **arr_pointer, int item)
{
	struct array *arr = *arr_pointer;

	if (arr->count >= arr->capacity)
	{
		int new_capacity = arr->capacity * GROWTH_FACTOR;

		struct array *new_arr = malloc(sizeof(*new_arr) + (sizeof(int) * new_capacity));

		if (new_arr == NULL)
		{
			return -1;
		}

		new_arr->count = arr->count;
		new_arr->capacity = new_capacity;

		int *new_data = (int *)(new_arr + 1);

		for (int i = 0; i < arr->count; ++i)
		{
			new_data[i] = array_get(arr, i);
		}

		free(arr);
		arr = new_arr;
		*arr_pointer = new_arr;
	}

	int *data = (int *)(arr + 1);
	data[arr->count] = item;

	++arr->count;

	return 0;
}

int main()
{
	struct array *my_array = init_array();

	if (my_array == NULL)
	{
		return 1;
	}

	printf("array len == %d\n", array_len(my_array));
	printf("array capacity = %d\n", my_array->capacity);

	for (int i = 0; i < 10; ++i)
	{
		if (array_append(&my_array, i) != 0)
		{
			free(my_array);
			return 1;
		};
	}

	printf("\nValues:\n");

	for (int i = 0; i < array_len(my_array); ++i)
	{
		printf("%d\n", array_get(my_array, i));
	}

	printf("array len == %d\n", array_len(my_array));
	printf("array capacity = %d\n", my_array->capacity);

	free(my_array);

	return 0;
}
