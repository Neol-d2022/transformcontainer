#include <stdio.h>
#include <string.h>

#include "transformcontainer.h"
#include "mm.h"

static void *duplicator(void *srcData, void *param)
{
    size_t l;

    l = strlen((char *)srcData);
    param = Mmalloc(l + 1);
    ((char *)param)[l] = '\0';
    memcpy(param, srcData, l);

    return param;
}

static void releaser(void *data, void *param)
{
    Mfree(param = data);
}

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
    TCCopyX(&dst, &tc, NULL, duplicator);
    printf("TCCopyX test (LL), count = %u, MDEBUG = %u\n", (unsigned int)TCCount(&dst), (unsigned int)MDebug());
    TCTravase(&dst, NULL, releaser);
    TCDeInit(&dst);

    TCTransform(&tc);
    TCInit(&dst);
    TCCopyX(&dst, &tc, NULL, duplicator);
    printf("TCCopyX test (LA), count = %u, MDEBUG = %u\n", (unsigned int)TCCount(&dst), (unsigned int)MDebug());
    TCTravase(&dst, NULL, releaser);
    TCDeInit(&dst);

    TCUndoTransform(&tc);
    TCInit(&dst);
    TCTransform(&dst);
    TCCopyX(&dst, &tc, NULL, duplicator);
    printf("TCCopyX test (AL), count = %u, MDEBUG = %u, contains = \n", (unsigned int)TCCount(&dst), (unsigned int)MDebug());
    for (j = 0; j < k; j += 1)
        printf("%s\n", (char *)TCI(&dst, j));
    TCTravase(&dst, NULL, releaser);
    TCDeInit(&dst);

    TCTransform(&tc);
    TCInit(&dst);
    TCTransform(&dst);
    TCCopyX(&dst, &tc, NULL, duplicator);
    printf("TCCopyX test (AA), count = %u, MDEBUG = %u, contains = \n", (unsigned int)TCCount(&dst), (unsigned int)MDebug());
    for (j = 0; j < k; j += 1)
        printf("%s\n", (char *)TCI(&dst, j));
    TCTravase(&dst, NULL, releaser);
    TCDeInit(&dst);

    TCDeInit(&tc);
    printf("Object destoryed, MDEBUG = %u\n", (unsigned int)MDebug());
    return 0;
}
