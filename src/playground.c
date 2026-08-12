#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>

#define DATA_CAPACITY 256
#define ITEM_CAPACITY 256

struct array_header
{
	size_t length;        // number of logical items
	size_t data_used;     // payload bytes currently occupied
	size_t data_capacity; // payload bytes allocated
	size_t item_capacity; // number of metadata entries allocated
};

struct array_item_header
{
	size_t offset;
	size_t size;
};

#define array_append(arr, arr_entries, item, item_size, item_align)                                \
	do                                                                                             \
	{                                                                                              \
		if ((arr) == NULL || (arr_entries) == NULL)                                                \
		{                                                                                          \
			/* TODO: free arr and arr_entries */                                                   \
			/* TODO: create arr and arr_entries */                                                 \
			fputs("array_append: arr and arr_entries must not be NULL\n", stderr);                 \
			break;                                                                                 \
		}                                                                                          \
                                                                                                   \
		struct array_header *header = (struct array_header *)(arr);                                \
                                                                                                   \
		struct array_item_header *item_header = (struct array_item_header *)(arr_entries);         \
                                                                                                   \
		unsigned char *data = (unsigned char *)(header + 1);                                       \
                                                                                                   \
		const size_t append_item_size = (item_size);                                               \
		const size_t append_item_align = (item_align);                                             \
                                                                                                   \
		if (append_item_align == 0)                                                                \
		{                                                                                          \
			fputs("array_append: item alignment must be greater than zero\n", stderr);             \
			break;                                                                                 \
		}                                                                                          \
                                                                                                   \
		/*                                                                                         \
		 * Calculate padding from the ACTUAL destination address.                                  \
		 *                                                                                         \
		 * current_address = data + data_used                                                      \
		 * padding makes the resulting address divisible by item_align.                            \
		 */                                                                                        \
		uintptr_t current_address = (uintptr_t)(data + header->data_used);                         \
                                                                                                   \
		size_t alignment_remainder = current_address % append_item_align;                          \
                                                                                                   \
		size_t padding = alignment_remainder == 0 ? 0 : append_item_align - alignment_remainder;   \
                                                                                                   \
		size_t destination_start = header->data_used + padding;                                    \
                                                                                                   \
		/* TODO: check integer overflow when calculating required size */                          \
                                                                                                   \
		/* TODO: Implement growth for array */                                                     \
		if (destination_start + append_item_size > header->data_capacity)                          \
		{                                                                                          \
			fputs("array_append: not enough data capacity\n", stderr);                             \
			break;                                                                                 \
		}                                                                                          \
                                                                                                   \
		/* TODO: Implement growth for array_entries */                                             \
		if (header->length >= header->item_capacity)                                               \
		{                                                                                          \
			fputs("array_append: not enough item metadata capacity\n", stderr);                    \
			break;                                                                                 \
		}                                                                                          \
                                                                                                   \
		/*                                                                                         \
		 * Copy the object's byte representation.                                                  \
		 * Struct padding contained inside item_size is copied as well.                            \
		 */                                                                                        \
		memcpy(data + destination_start, (item), append_item_size);                                \
                                                                                                   \
		/* Store metadata for this logical array element. */                                       \
		size_t length = header->length;                                                            \
                                                                                                   \
		item_header[length].offset = destination_start;                                            \
		item_header[length].size = append_item_size;                                               \
                                                                                                   \
		/* Update array state. */                                                                  \
		header->length += 1;                                                                       \
		header->data_used = destination_start + append_item_size;                                  \
                                                                                                   \
	} while (0)

void *array_get(struct array_header *header, struct array_item_header *item_header, size_t index)
{
	struct array_item_header *item = item_header + index;
	// Array Data
	unsigned char *data = (unsigned char *)(header + 1);

	return data + item->offset;
}

// void array_remove(struct array_header_t *array_header,
//                   struct array_item_header_t *array_item_header, size_t index)
// {
// 	assert(index < array_header->length && "index out of bound");
//
// 	struct array_item_header_t *item_meta_data = array_item_header + index;
//
// 	unsigned char *array_data = (unsigned char *)(array_header + 1);
//
// 	size_t removed_offset = item_meta_data->offset;
//
// 	size_t removed_size = item_meta_data->size;
//
// 	size_t item_end = removed_offset + removed_size;
//
// 	size_t bytes_to_move = array_header->current_offset - item_end;
//
// 	memmove(array_data + removed_offset, array_data + item_end, bytes_to_move);
//
// 	for (size_t i = index; i + 1 < array_header->length; ++i)
// 	{
// 		array_item_header[i] = array_item_header[i + 1];
//
// 		array_item_header[i].offset -= removed_size;
// 	}
//
// 	array_header->current_offset -= removed_size;
//
// 	array_header->length -= 1;
// }

int main()
{
	// Init Array Header
	struct array_header *arr = malloc(sizeof(*arr) + sizeof(unsigned char) * DATA_CAPACITY);

	arr->length = 0;
	arr->data_used = 0;
	arr->data_capacity = DATA_CAPACITY;
	arr->item_capacity = ITEM_CAPACITY;

	// Init Array Items header
	struct array_item_header *arr_entries = malloc(sizeof(*arr_entries) * ITEM_CAPACITY);

	char item[] = "skills";
	// char *item2= "newfoundland";

	size_t item_size = strlen(item) + 1;

	int item1 = 676;
	float item2 = 212.0;

	array_append(arr, arr_entries, item, item_size, alignof(char));

	array_append(arr, arr_entries, &item1, sizeof(item1), alignof(int));

	array_append(arr, arr_entries, &item2, sizeof(item2), alignof(float));

	char *arr_item = (char *)array_get(arr, arr_entries, 0);
	int *arr_item1 = (int *)array_get(arr, arr_entries, 1);
	float *arr_item2 = (float *)array_get(arr, arr_entries, 2);
	printf("%s\n", arr_item);
	printf("%d\n", *arr_item1);
	printf("%f\n", *arr_item2);

	// char *item2= "newfoundland";
	// char *item3 = "praise";
	// char *item4 = "what is this";
	// char *item5 = "testing";
	// char *item6 = "I hope it works";

	// char *items[] = {item, item2, item3, item4, item5, item6};
	//
	// for (int i = 0; i < 6; ++i){
	//     array_append(
	//         array_header,
	//         array_item_header,
	//         items[i],
	//         strlen(items[i]) + 1
	//     );
	// }
	//
	// for (size_t i = 0; i < array_header->length; ++i){
	//     char *arr_item = (char *)array_get(
	//         array_header,
	//         array_item_header,
	//         i
	//     );
	//     printf("%s\n", arr_item);
	// }
	//
	// printf("\n");
	//
	// array_remove(
	//         array_header,
	//         array_item_header,
	//         0
	//     );
	//
	// // array_remove(
	// //         array_header,
	// //         array_item_header,
	// //         array_header->length - 1
	// //     );
	//
	// for (size_t i = 0; i < array_header->length; ++i){
	//     char *arr_item = (char *)array_get(
	//         array_header,
	//         array_item_header,
	//         i
	//     );
	//     printf("%s\n", arr_item);
	// }

	free(arr);
	free(arr_entries);
	return 0;
}
