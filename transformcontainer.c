#include <stdlib.h>
#include <string.h>

#include "mm.h"
#include "transformcontainer.h"

#define _TC_TRANSFORMED_FLAG 0x01

// =====================
// Internal object types
typedef struct
{
    void **target;
    size_t index;
} _TransformTraverser_t;

// =========================
// Local function definitions
static void _TCClearLinkedList(TC_t *tc);
static void _TCClearArray(TC_t *tc);
static void _TCTransformTraverser(void *data, void *param);
static void _TCUndoTransformTraverser(void *data, void *param);
static void _TCTravaseL(TC_t *tc, void *param, void (*handler)(void *data, void *param));
static void _TCTravaseA(TC_t *tc, void *param, void (*handler)(void *data, void *param));

// ================
// Exported functions
void TCInit(TC_t *tc)
{
    memset(tc, 0, sizeof(*tc));
    tc->lastNodeLinkPtr = &(tc->variableHead);
}

void TCDeInit(TC_t *tc)
{
#ifdef _TC_PRODUCTION
    if (tc->fixed)
        _TCClearLinkedList(tc);
    else
        _TCClearArray(tc);
#else
    if (!TCIsTransformed(tc))
        _TCClearLinkedList(tc);
    else
        _TCClearArray(tc);

#ifdef _TC_DEBUG
    memset(tc, 0xab, sizeof(*tc));
#endif // #ifdef _TC_DEBUG

#endif // #ifdef _TC_PRODUCTION
}

int TCAdd(TC_t *tc, void *data)
{
    TC_Node_t **pp, *n;

#ifndef _TC_PRODUCTION
    if (TCIsTransformed(tc))
        return 1;
#endif

    pp = tc->lastNodeLinkPtr;
    n = (TC_Node_t *)Mmalloc(sizeof(*n));
    n->data = data;
    n->next = *pp;
    *pp = n;
    tc->lastNodeLinkPtr = &(n->next);
    tc->variableCount += 1;

    return 0;
}

int TCTransform(TC_t *tc)
{
    _TransformTraverser_t io;
    void **a;
    size_t n;

#ifndef _TC_PRODUCTION
    if (TCIsTransformed(tc))
        return 1;
#endif

    n = tc->variableCount;
    a = (void **)Mmalloc(sizeof(*a) * n);
    io.target = a;
    io.index = 0;
    _TCTravaseL(tc, &io, _TCTransformTraverser);
    _TCClearLinkedList(tc);
    tc->fixed = a;
    tc->fixedCount = n;
    tc->variableHead = NULL;
    tc->variableCount = 0;
    tc->flags |= _TC_TRANSFORMED_FLAG;

    return 0;
}

bool TCIsTransformed(TC_t *tc)
{
    if (tc->flags & _TC_TRANSFORMED_FLAG)
        return true;
    else
        return false;
}

size_t TCCount(TC_t *tc)
{
#ifdef _TC_PRODUCTION
    return tc->fixedCount + tc->variableCount;
#else
    if (TCIsTransformed(tc))
        return tc->fixedCount;
    else
        return tc->variableCount;
#endif
}

void *TCI(TC_t *tc, size_t i)
{
#ifndef _TC_PRODUCTION
    if (!TCIsTransformed(tc))
        abort();
    if (i >= tc->fixedCount)
        abort();
#endif
    return (tc->fixed)[i];
}

void TCTravase(TC_t *tc, void *param, void (*handler)(void *data, void *param))
{

#ifdef _TC_PRODUCTION
    if (tc->fixed)
        _TCTravaseA(tc, param, handler);
    else
        _TCTravaseL(tc, param, handler);
#else
    if (TCIsTransformed(tc))
        _TCTravaseA(tc, param, handler);
    else
        _TCTravaseL(tc, param, handler);
#endif
}

int TCUndoTransform(TC_t *tc)
{
#ifndef _TC_PRODUCTION
    if (!TCIsTransformed(tc))
        return 1;
#endif

    tc->flags &= !_TC_TRANSFORMED_FLAG;
    tc->variableHead = NULL;
    tc->lastNodeLinkPtr = &(tc->variableHead);
    tc->variableCount = 0;
    _TCTravaseA(tc, tc, _TCUndoTransformTraverser);
    _TCClearArray(tc);
    tc->fixed = NULL;
    tc->fixedCount = 0;

    return 0;
}

// =================
// Private functions
static void _TCClearLinkedList(TC_t *tc)
{
    TC_Node_t *c, *d;

    c = tc->variableHead;

    while (c)
    {
        d = c->next;
        Mfree(c);
        c = d;
    }
}

static void _TCClearArray(TC_t *tc)
{
    Mfree(tc->fixed);
}

static void _TCTransformTraverser(void *data, void *param)
{
    _TransformTraverser_t *io = (_TransformTraverser_t *)param;

    (io->target)[io->index++] = data;
}

static void _TCUndoTransformTraverser(void *data, void *param)
{
    TC_t *tc = (TC_t *)param;

    TCAdd(tc, data);
}

static void _TCTravaseL(TC_t *tc, void *param, void (*handler)(void *data, void *param))
{
    TC_Node_t *c = tc->variableHead;

    while (c)
    {
        handler(c->data, param);
        c = c->next;
    }
}

static void _TCTravaseA(TC_t *tc, void *param, void (*handler)(void *data, void *param))
{
    size_t i, n;

    n = tc->fixedCount;

    for (i = 0; i < n; i += 1)
        handler((tc->fixed)[i], param);
}
