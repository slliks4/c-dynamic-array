#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

struct array_header_t{
    size_t current_offset;
    size_t length;
    size_t capacity;
};

struct array_item_header_t{
    size_t offset;
};

#define ARRAY_CAPACITY 3200

// TODO: Merge Item header and array_header to be in on single memory
void array_append(
        struct array_header_t *array_header,
        struct array_item_header_t *array_item_header,
        char *item,
        size_t item_size
        ){
    // Array Data 
    unsigned char *array_data = (unsigned char *)(array_header + 1);

    // Check array capacity
    // TODO: grow array
    assert(
            array_header->current_offset + item_size <=
            array_header->capacity
            );

    // copy item to array_data
    memcpy(
        array_data + array_header->current_offset,
        item,
        item_size
    );

    // Update item header array
    // Check array items header capacity
    // TODO: Grow array item header
    assert(
            array_header->length <= array_header->capacity
            );

    (array_item_header + array_header->length)->offset =
        array_header->current_offset;


    // update array_header
    array_header->length += 1;
    array_header->current_offset += item_size;
}

unsigned char * array_get(
        struct array_header_t *array_header,
        struct array_item_header_t *array_item_header,
        size_t index
        ){
    struct array_item_header_t *item = array_item_header + index;
    // Array Data 
    unsigned char *array_data = (unsigned char *)(array_header + 1);

    return array_data + item->offset;
}


int main(){
    // Init Array Header
    struct array_header_t *array_header = malloc(
            sizeof(*array_header) + sizeof(unsigned char) * ARRAY_CAPACITY
            );

    array_header->current_offset = 0;
    array_header->length = 0;
    array_header->capacity = ARRAY_CAPACITY;

    // Init Array Items header
    struct array_item_header_t *array_item_header = malloc(
            sizeof(*array_item_header) * ARRAY_CAPACITY
            );

    char item[] = "skills";
    char *item2= "newfoundland";
    char *item3 = "praise";
    char *item4 = "what is this";
    char *item5 = "testing";
    char *item6 = "I hope it works";

    char *items[] = {item, item2, item3, item4, item5, item6};

    for (int i = 0; i < 6; ++i){
        array_append(
            array_header,
            array_item_header,
            items[i],
            strlen(items[i]) + 1
        );
    }

    for (int i = 0; i < 6; ++i){
        char *item = (char *)array_get(
            array_header,
            array_item_header,
            i
        );
        printf("%s\n",item);
    }


    free(array_header);
    free(array_item_header);
    return 0;
}
