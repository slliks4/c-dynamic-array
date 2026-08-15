#include <assert.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_CAPACITY 2
#define ITEM_CAPACITY 1
#define GROWTH_FACTOR 2

/*
 * ============================================================
 * NEXT STEPS / DESIGN NOTES
 * ============================================================
 *
 * CURRENT DESIGN
 *
 * array_header allocation:
 *
 *     [ array_header ][ packed array data ................. ]
 *
 * array_header->item_header points to a separate allocation:
 *
 *     [ item_header ][ item_header ][ item_header ] ...
 *
 *
 * LOOKUP
 *
 *     logical index
 *          ↓
 *     item_header[index]
 *          ↓
 *        offset
 *          ↓
 *     array_data + offset
 *
 *
 * ------------------------------------------------------------
 * TODO 1: REWORK DATA GROWTH / REALLOCATION
 * ------------------------------------------------------------
 *
 * Current realloc implementation has an important problem:
 *
 * `data`, alignment calculations, padding, and destination_start
 * are calculated BEFORE realloc().
 *
 * realloc() may move the array allocation to a different address.
 * If this happens:
 *
 *     - `header` changes
 *     - array data base address changes
 *     - previous `data` pointer becomes invalid
 *     - previous alignment calculation may no longer be valid
 *     - previous destination_start may no longer satisfy alignment
 *
 * Possible next implementation:
 *
 * Instead of directly reallocating the packed data allocation:
 *
 *     1. Calculate required new capacity.
 *     2. malloc() a new array allocation.
 *     3. Copy/rebuild the array header.
 *     4. Determine the NEW payload base address.
 *     5. Walk through each logical item.
 *     6. Recalculate padding for that item's alignment using
 *        the new base address.
 *     7. Copy the old item's bytes into the new aligned location.
 *     8. Update that item's metadata offset.
 *     9. Update data_used as items are repacked.
 *    10. Only after the entire move succeeds:
 *            free(old_arr);
 *            arr = new_arr;
 *
 * This would effectively COMPACT + REALIGN the entire payload
 * whenever the data allocation grows.
 *
 *
 * IMPORTANT:
 *
 * In order to rebuild/repack old items while preserving their
 * alignment, array_item_header will probably need to remember
 * each item's required alignment.
 *
 * Possible future metadata:
 *
 *     offset
 *     size
 *     alignment
 *
 * Do not implement blindly yet — explore whether alignment must
 * remain stored permanently or whether another design is better.
 *
 *
 * ------------------------------------------------------------
 * TODO 2: CAPACITY GROWTH
 * ------------------------------------------------------------
 *
 * Data capacity and item-header capacity are independent:
 *
 *     data_capacity -> bytes
 *     item_capacity -> number of logical item metadata entries
 *
 * Grow them independently.
 *
 * Data growth may require repacking because addresses/alignment
 * can change.
 *
 * item_header growth does NOT require moving array payload data
 * because item_header currently lives in a separate allocation.
 *
 * Also consider:
 *
 *     - capacity == 0
 *     - a single appended item larger than one growth step
 *     - size_t overflow when calculating allocation sizes
 *
 *
 * ------------------------------------------------------------
 * TODO 3: POINTER INVALIDATION POLICY
 * ------------------------------------------------------------
 *
 * array_get() returns a pointer directly into internal array data.
 *
 * Example:
 *
 *     int *p = array_get(arr, 1);
 *
 * `p` is a PHYSICAL pointer to the bytes at the current location.
 *
 * If metadata later changes:
 *
 *     array_get(arr, 1)
 *
 * may resolve to a different logical item, while `p` still points
 * to the old physical bytes.
 *
 * If the payload allocation is moved/rebuilt:
 *
 *     ALL previously returned pointers into the old payload
 *     become invalid/dangling.
 *
 * Need to decide/document a policy such as:
 *
 *     "Pointers returned by array_get() remain valid only until
 *      an operation that relocates/rebuilds the payload."
 *
 * Similar to pointer/reference invalidation rules in other
 * dynamic containers.
 *
 *
 * ------------------------------------------------------------
 * TODO 4: REMOVE POLICY
 * ------------------------------------------------------------
 *
 * Current remove implementation performs LOGICAL REMOVAL only.
 *
 * It shifts item_header entries so the removed item is no longer
 * reachable by its logical index.
 *
 * It DOES NOT currently:
 *
 *     - remove bytes from array_data
 *     - compact payload data
 *     - reclaim removed item's space
 *     - decrease data_used
 *
 * Consequence:
 *
 *     old pointer returned by array_get()
 *              ↓
 *         still points to old bytes
 *
 * even though that item has been removed logically.
 *
 * This creates internal fragmentation/dead space.
 *
 *
 * Possible temporary experiment:
 *
 *     memset() removed bytes after removing the metadata entry.
 *
 * BUT:
 *
 *     memset(..., 0, size)
 *
 * should NOT be treated as a generic guarantee that an arbitrary
 * C object now has the logical value "zero".
 *
 * An all-zero byte representation is not something this generic
 * container should assume represents every possible type/value.
 *
 * Zeroing can instead be considered:
 *
 *     - debug hygiene
 *     - making stale raw bytes visibly overwritten
 *     - an experiment while deciding the real removal policy
 *
 * It does NOT solve pointer invalidation.
 *
 *
 * Future removal approaches to explore:
 *
 * A. Logical deletion only
 *      + simple
 *      + existing object addresses remain stable
 *      - fragmentation
 *
 * B. Compact payload immediately
 *      + reclaim memory
 *      - O(n)
 *      - objects move
 *      - old pointers may become invalid
 *      - alignment must be recalculated
 *
 * C. Keep holes and reuse them later
 *      + avoids moving everything
 *      - requires free-space metadata
 *      - starts becoming allocator-like
 *
 *
 * ------------------------------------------------------------
 * TODO 5: REMOVE + ALIGNMENT
 * ------------------------------------------------------------
 *
 * If physical compaction is implemented later, DO NOT simply
 * shift arbitrary bytes left by removed_size.
 *
 * Example:
 *
 *     [ char data ][padding][int][padding][struct]
 *
 * Removing an earlier object changes candidate addresses for all
 * following objects.
 *
 * Every moved object may need its destination recalculated using:
 *
 *     actual destination address
 *     size
 *     required alignment
 *
 * This is another reason item metadata may eventually need to
 * store alignment.
 *
 *
 * ------------------------------------------------------------
 * TODO 6: ARRAY_GET
 * ------------------------------------------------------------
 *
 * Improve array_get() tomorrow:
 *
 *     - validate arr != NULL
 *     - validate item_header != NULL
 *     - validate index < length
 *     - document pointer invalidation semantics
 *
 * Potential future API:
 *
 *     array_get()      -> raw pointer
 *     array_item_size() -> stored byte size
 *
 * Caller remains responsible for interpreting/casting the raw
 * bytes as the correct type.
 *
 *
 * ------------------------------------------------------------
 * TODO 7: ARRAY_REMOVE BOUNDS CHECK
 * ------------------------------------------------------------
 *
 * Current index check needs review.
 *
 * index is size_t, therefore:
 *
 *     index < 0
 *
 * is not meaningful because size_t is unsigned.
 *
 * Also revisit the exact upper-bound condition.
 *
 * Do this while cleaning array_remove().
 *
 *
 * ------------------------------------------------------------
 * TODO 8: ARRAY DESTRUCTION
 * ------------------------------------------------------------
 *
 * The array currently owns TWO allocations:
 *
 *     arr
 *     arr->item_header
 *
 * Calling only:
 *
 *     free(arr);
 *
 * does not release item_header.
 *
 * Introduce something like array_free()/array_destroy() later
 * so ownership is centralized.
 *
 *
 * ------------------------------------------------------------
 * TODO 9: OPTIONAL FUTURE SINGLE-ALLOCATION DESIGN
 * ------------------------------------------------------------
 *
 * Current two-allocation design is useful because:
 *
 *     payload grows independently
 *     metadata grows independently
 *
 * Later experiment with:
 *
 *     [header][metadata region][payload region]
 *
 * only after the current design is stable.
 *
 * That introduces another problem:
 *
 *     metadata growth and payload growth can collide.
 *
 * Leave this until later.
 * ============================================================
 */

struct array_item_header
{
	size_t offset;
	size_t size;

	/*
	 * TODO:
	 * Consider storing alignment here if data growth/removal
	 * rebuilds objects at new addresses.
	 *
	 * size_t alignment;
	 */
};

struct array_header
{
	size_t length;        // Number of logical items.
	size_t data_used;     // End of currently used payload region in bytes.
	size_t data_capacity; // Total payload capacity in bytes.
	size_t item_capacity; // Number of metadata entries allocated.

	/*
	 * Separate allocation containing the fixed-size lookup metadata.
	 */
	struct array_item_header *item_header;
};

struct array_header *array_init(void)
{
	/*
	 * Main allocation:
	 *
	 *     [array_header][raw payload bytes...]
	 */
	struct array_header *arr = malloc(sizeof(*arr) + sizeof(unsigned char) * DATA_CAPACITY);

	if (arr == NULL)
	{
		return NULL;
	}

	/*
	 * Metadata allocation.
	 *
	 * TODO:
	 * Keep this independent during the next growth implementation.
	 */
	struct array_item_header *arr_entries = malloc(sizeof(*arr_entries) * ITEM_CAPACITY);

	if (arr_entries == NULL)
	{
		free(arr);
		return NULL;
	}

	arr->length = 0;
	arr->data_used = 0;
	arr->data_capacity = DATA_CAPACITY;
	arr->item_capacity = ITEM_CAPACITY;
	arr->item_header = arr_entries;

	return arr;
}

/*
 * TODO: ARRAY_APPEND
 *
 * Main thing to revisit tomorrow:
 *
 * The current code calculates:
 *
 *     data
 *     current_address
 *     alignment_remainder
 *     padding
 *     destination_start
 *
 * BEFORE potentially reallocating the payload.
 *
 * If realloc moves `header`, those calculations belong to the
 * old address.
 *
 * Explore replacing payload realloc with:
 *
 *     malloc new payload
 *          ↓
 *     rebuild/repack each item with alignment
 *          ↓
 *     update each metadata offset
 *          ↓
 *     free old payload only after success
 *
 * Keep item_header growth independent.
 */
#define array_append(arr, item, item_size, item_align)                                             \
	do                                                                                             \
	{                                                                                              \
		if ((arr) == NULL)                                                                         \
		{                                                                                          \
			/* Init Array Header*/                                                                 \
			(arr) = array_init();                                                                  \
			if ((arr) == NULL)                                                                     \
			{                                                                                      \
				fputs("Error: malloc failed to initialize memory\n", stderr);                      \
				break;                                                                             \
			}                                                                                      \
		}                                                                                          \
                                                                                                   \
		struct array_header *header = (struct array_header *)(arr);                                \
		struct array_item_header *item_header = header->item_header;                               \
                                                                                                   \
		if (item_header == NULL)                                                                   \
		{                                                                                          \
			fputs("Error: Invalid array_header \n", stderr);                                       \
			break;                                                                                 \
		}                                                                                          \
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
		if (destination_start + append_item_size > header->data_capacity)                          \
		{                                                                                          \
			size_t new_capacity = header->data_capacity * GROWTH_FACTOR;                           \
			size_t new_allocation_size = sizeof(*header) + new_capacity;                           \
			/*                                                                                     \
			 * Use a temporary pointer.                                                            \
			 *                                                                                     \
			 * If realloc fails, it returns NULL but the original allocation remains               \
			 * valid. Assigning directly to header would lose the original pointer.                \
			 */                                                                                    \
			struct array_header *temporary_header = realloc(header, new_allocation_size);          \
                                                                                                   \
			if (temporary_header == NULL)                                                          \
			{                                                                                      \
				perror("array_append: realloc");                                                   \
				break;                                                                             \
			}                                                                                      \
                                                                                                   \
			header = temporary_header;                                                             \
			header->data_capacity = new_capacity;                                                  \
                                                                                                   \
			/* realloc may move the allocation, so update the array pointer. */                    \
			(arr) = (void *)(header);                                                              \
		}                                                                                          \
                                                                                                   \
		if (header->length >= header->item_capacity)                                               \
		{                                                                                          \
			size_t new_capacity = header->item_capacity * GROWTH_FACTOR;                           \
			size_t new_allocation_size = sizeof(*item_header) * new_capacity;                      \
			/*                                                                                     \
			 * Use a temporary pointer.                                                            \
			 *                                                                                     \
			 * If realloc fails, it returns NULL but the original allocation remains               \
			 * valid. Assigning directly to header would lose the original pointer.                \
			 */                                                                                    \
			struct array_item_header *temporary_item_header =                                      \
			    realloc(item_header, new_allocation_size);                                         \
                                                                                                   \
			if (temporary_item_header == NULL)                                                     \
			{                                                                                      \
				perror("array_append: realloc");                                                   \
				break;                                                                             \
			}                                                                                      \
                                                                                                   \
			item_header = temporary_item_header;                                                   \
			header->item_capacity = new_capacity;                                                  \
                                                                                                   \
			/* realloc may move the allocation, so update the array pointer. */                    \
			header->item_header = (void *)(item_header);                                           \
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

/*
 * array_get() returns a raw pointer into the array's INTERNAL
 * payload allocation.
 *
 * IMPORTANT:
 *
 * The returned pointer is a physical address, not a permanent
 * logical reference to an array index.
 *
 * If the array allocation later moves/rebuilds, previously
 * returned pointers can become invalid.
 *
 * TODO:
 *     - bounds checking
 *     - NULL checking
 *     - document exact invalidation policy
 */
void *array_get(struct array_header *arr, size_t index)
{
	struct array_item_header *item_header = arr->item_header;
	struct array_item_header *item = item_header + index;

	unsigned char *data = (unsigned char *)(arr + 1);

	return data + item->offset;
}

/*
 * CURRENT REMOVE SEMANTICS:
 *
 * Logical removal only.
 *
 * Metadata entry disappears from the logical array, but payload
 * bytes remain physically present.
 *
 * Therefore a pointer obtained BEFORE remove may continue pointing
 * at the old bytes even though array_get(index) now resolves through
 * different metadata.
 *
 * TODO tomorrow:
 *
 *     1. Fix/review bounds checking.
 *     2. Decide whether removed payload bytes should temporarily
 *        be overwritten for debugging.
 *     3. Do NOT treat memset(..., 0, size) as a generic destructor
 *        or universal representation of a zero-valued object.
 *     4. Leave physical compaction until alignment-aware repacking
 *        has been designed.
 *     5. Decide long-term fragmentation/removal policy.
 */
#define array_remove(arr, index)                                                                   \
	do                                                                                             \
	{                                                                                              \
		if ((arr) == NULL)                                                                         \
		{                                                                                          \
			fputs("Error: malloc failed to initialize memory\n", stderr);                          \
			break;                                                                                 \
		}                                                                                          \
                                                                                                   \
		struct array_header *header = (struct array_header *)(arr);                                \
		if (index > header->length || index < 0)                                                   \
		{                                                                                          \
			perror("out of bound");                                                                \
			break;                                                                                 \
		}                                                                                          \
                                                                                                   \
		struct array_item_header *item_header = header->item_header;                               \
		if (item_header == NULL)                                                                   \
		{                                                                                          \
			fputs("Error: Invalid array_header \n", stderr);                                       \
			break;                                                                                 \
		}                                                                                          \
                                                                                                   \
		for (size_t i = index; i < header->length - 1; ++i)                                        \
		{                                                                                          \
			item_header[i] = item_header[i + 1];                                                   \
		}                                                                                          \
		header->length -= 1;                                                                       \
	} while (0)

/*
 * TODO:
 * Replace the final free(arr) usage with an array destruction
 * function because arr->item_header is separately allocated.
 */

int main()
{
	void *arr = NULL;
	char item[] = "skills";
	// char *item2= "newfoundland";

	size_t item_size = strlen(item) + 1;

	int item1 = 676;
	float item2 = 212.0;

	array_append(arr, item, item_size, alignof(char));

	array_append(arr, &item1, sizeof(item1), alignof(int));

	array_append(arr, &item2, sizeof(item2), alignof(float));

	char *arr_item = (char *)array_get(arr, 0);
	int *arr_item1 = (int *)array_get(arr, 1);
	float *arr_item2 = (float *)array_get(arr, 2);
	printf("%s\n", arr_item);
	printf("%d\n", *arr_item1);
	printf("%f\n", *arr_item2);

	array_remove(arr, 1);

	printf("\n");
	int *arr_item_test = (int *)array_get(arr, 1);
	printf("%d\n", *arr_item_test);

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

	// array_remove(
	//         array_header,
	//         array_item_header,
	//         array_header->length - 1
	//     );

	// for (size_t i = 0; i < array_header->length; ++i){
	//     char *arr_item = (char *)array_get(
	//         array_header,
	//         array_item_header,
	//         i
	//     );
	//     printf("%s\n", arr_item);
	// }

	free(arr);
	return 0;
}
