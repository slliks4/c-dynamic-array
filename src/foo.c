#include <stdio.h>
#include <string.h>
#include <stdalign.h>

#define testing(item) \
    _Alignof(item)

int main(){
    int num = 2;
    char *s = "testing";

    printf("%zu\n",testing(num));
    printf("%zu\n",testing(s));
    printf("%zu\n",testing(*s));
    return 0;
}
