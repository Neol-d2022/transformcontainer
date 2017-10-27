#include <stdio.h>

#include "transformcontainer.h"

/* Test script */
int main(int argc, char **argv)
{
    TC_t tc;
    int i;
    size_t j, k;

    TCInit(&tc);
    printf("Object initialized\n");
    for (i = 0; i < argc; i += 1)
        TCAdd(&tc, argv[i]);
    printf("Data stored\n");

    printf("Before transform, count = %u\n", (unsigned int)TCCount(&tc));
    i = TCTransform(&tc);
    k = TCCount(&tc);
    printf("After transform , count = %u, result = %i, flag = 0x%08u, contains = \n", (unsigned int)k, i, (unsigned int)tc.flags);
    for (j = 0; j < k; j += 1)
        printf("%s\n", (char *)TCI(&tc, j));
    i = TCUndoTransform(&tc);
    printf("Undo transform  , count = %u, result = %i, flag = 0x%08u\n", (unsigned int)TCCount(&tc), i, (unsigned int)tc.flags);
    TCAdd(&tc, "Test 1");
    TCAdd(&tc, "Test 2");
    TCAdd(&tc, "Test 3");
    printf("Data stored\n");

    i = TCTransform(&tc);
    k = TCCount(&tc);
    printf("After transform , count = %u, result = %i, flag = 0x%08u, contains = \n", (unsigned int)k, i, (unsigned int)tc.flags);
    for (j = 0; j < k; j += 1)
        printf("%s\n", (char *)TCI(&tc, j));

    TCDeInit(&tc);
    printf("Object destoryed\n");
    return 0;
}
