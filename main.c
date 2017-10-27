#include <stdio.h>

#include "transformcontainer.h"
#include "mm.h"

/* Test script */
int main(int argc, char **argv)
{
    TC_t tc, dst;
    int i;
    size_t j, k;

    TCInit(&tc);
    printf("Object initialized, MDEBUG = %u\n", (unsigned int)MDebug());
    for (i = 0; i < argc; i += 1)
        TCAdd(&tc, argv[i]);
    printf("Data stored\n");

    printf("Before transform, count = %u, MDEBUG = %u\n", (unsigned int)TCCount(&tc), (unsigned int)MDebug());
    i = TCTransform(&tc);
    k = TCCount(&tc);
    printf("After transform , count = %u, result = %i, flag = 0x%08u, MDEBUG = %u, contains = \n", (unsigned int)k, i, (unsigned int)tc.flags, (unsigned int)MDebug());
    for (j = 0; j < k; j += 1)
        printf("%s\n", (char *)TCI(&tc, j));

    i = TCUndoTransform(&tc);
    printf("Undo transform  , count = %u, result = %i, flag = 0x%08u, MDEBUG = %u\n", (unsigned int)TCCount(&tc), i, (unsigned int)tc.flags, (unsigned int)MDebug());
    TCAdd(&tc, "Test 1");
    TCAdd(&tc, "Test 2");
    TCAdd(&tc, "Test 3");
    printf("Data stored\n");

    i = TCTransform(&tc);
    k = TCCount(&tc);
    printf("After transform , count = %u, result = %i, flag = 0x%08u, MDEBUG = %u, contains = \n", (unsigned int)k, i, (unsigned int)tc.flags, (unsigned int)MDebug());
    for (j = 0; j < k; j += 1)
        printf("%s\n", (char *)TCI(&tc, j));

    TCUndoTransform(&tc);
    TCInit(&dst);
    TCCopy(&dst, &tc);
    printf("TCCopy test (LL), count = %u, MDEBUG = %u\n", (unsigned int)TCCount(&dst), (unsigned int)MDebug());
    TCDeInit(&dst);

    TCTransform(&tc);
    TCInit(&dst);
    TCCopy(&dst, &tc);
    printf("TCCopy test (LA), count = %u, MDEBUG = %u\n", (unsigned int)TCCount(&dst), (unsigned int)MDebug());
    TCDeInit(&dst);

    TCUndoTransform(&tc);
    TCInit(&dst);
    TCAdd(&dst, "Padding");
    TCTransform(&dst);
    TCCopy(&dst, &tc);
    printf("TCCopy test (AL), count = %u, MDEBUG = %u, contains = \n", (unsigned int)TCCount(&dst), (unsigned int)MDebug());
    for (j = 0; j < k + 1; j += 1)
        printf("%s\n", (char *)TCI(&dst, j));
    TCDeInit(&dst);

    TCTransform(&tc);
    TCInit(&dst);
    TCAdd(&dst, "Padding");
    TCTransform(&dst);
    TCCopy(&dst, &tc);
    printf("TCCopy test (AA), count = %u, MDEBUG = %u, contains = \n", (unsigned int)TCCount(&dst), (unsigned int)MDebug());
    for (j = 0; j < k + 1; j += 1)
        printf("%s\n", (char *)TCI(&dst, j));
    TCDeInit(&dst);

    TCDeInit(&tc);
    printf("Object destoryed, MDEBUG = %u\n", (unsigned int)MDebug());
    return 0;
}
