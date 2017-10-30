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

typedef struct
{
    TC_t *dst;
    void *param;
    void *(*dataDuplicator)(void *soucreData, void *param);
} _TCCopyX_internal_object_t;

// =========================
// Local function definitions
static void _TCClearLinkedList(TC_t *tc);
static void _TCClearArray(TC_t *tc);
static void _TCTransformTraverser(void *data, void *param);
static void _TCUndoTransformTraverser(void *data, void *param);
static void _TCTravaseL(TC_t *tc, void *param, void (*handler)(void *data, void *param));
static void _TCTravaseA(TC_t *tc, void *param, void (*handler)(void *data, void *param));
static void _TCCopyXALTraverserC(void *data, void *param);
static void _TCCopyXLXTraverserC(void *data, void *param);
static void _TCCopyXALTraverserD(void *data, void *param);
static void _TCCopyXLXTraverserD(void *data, void *param);

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
    if (tc->flags & _TC_TRANSFORMED_FLAG)
#else
    if (TCIsTransformed(tc))
#endif
        _TCClearArray(tc);
    else
        _TCClearLinkedList(tc);

#ifdef _TC_DEBUG
    memset(tc, 0xab, sizeof(*tc));
#endif
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
    if (n == 0)
        a = (void **)Mmalloc(sizeof(*a) * 1);
    else
        a = (void **)Mmalloc(sizeof(*a) * n);
    io.target = a;
    io.index = 0;
    _TCTravaseL(tc, &io, _TCTransformTraverser);
    _TCClearLinkedList(tc);
    tc->fixed = a;
    tc->fixedCount = n;
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
    if (tc->flags & _TC_TRANSFORMED_FLAG)
#else
    if (TCIsTransformed(tc))
#endif
        _TCTravaseA(tc, param, handler);
    else
        _TCTravaseL(tc, param, handler);
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
    tc->fixedCount = 0;

    return 0;
}

void TCCopy(TC_t *dst, TC_t *src)
{
    TCCopyX(dst, src, NULL, NULL);
}

void TCCopyX(TC_t *dst, TC_t *src, void *param, void *(*dataDuplicator)(void *soucreData, void *param))
{
    _TCCopyX_internal_object_t io;
    size_t i, n;

#ifndef _TC_PRODUCTION
    if (TCIsTransformed(dst))
#else
    if (dst->flags & _TC_TRANSFORMED_FLAG)
#endif
    {
#ifndef _TC_PRODUCTION
        if (TCIsTransformed(src))
#else
        if (src->flags & _TC_TRANSFORMED_FLAG)
#endif
        {
            n = dst->fixedCount + src->fixedCount;
            dst->fixed = (void **)Mrealloc(dst->fixed, sizeof(*(dst->fixed)) * n);
            if (dataDuplicator)
                for (i = 0; i < src->fixedCount; i += 1)
                    (dst->fixed)[i + dst->fixedCount] = dataDuplicator((src->fixed)[i], param);
            else
                memcpy(dst->fixed + dst->fixedCount, src->fixed, sizeof(*(dst->fixed)) * src->fixedCount);
            dst->fixedCount = n;
        }
        else
        {
            dst->fixed = (void **)Mrealloc(dst->fixed, sizeof(*(dst->fixed)) * (dst->fixedCount + src->variableCount));
            io.dst = dst;
            if (dataDuplicator)
            {
                io.param = param;
                io.dataDuplicator = dataDuplicator;
                _TCTravaseL(src, &io, _TCCopyXALTraverserD);
            }
            else
                _TCTravaseL(src, &io, _TCCopyXALTraverserC);
        }
    }
    else
    {
        io.dst = dst;
        if (dataDuplicator)
        {
            io.param = param;
            io.dataDuplicator = dataDuplicator;
            TCTravase(src, &io, _TCCopyXLXTraverserD);
        }
        else
            TCTravase(src, &io, _TCCopyXLXTraverserC);
    }
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

static void _TCCopyXALTraverserC(void *data, void *param)
{
    _TCCopyX_internal_object_t *io = (_TCCopyX_internal_object_t *)param;

    io->dst->fixed[io->dst->fixedCount++] = data;
}

static void _TCCopyXLXTraverserC(void *data, void *param)
{
    _TCCopyX_internal_object_t *io = (_TCCopyX_internal_object_t *)param;

    TCAdd(io->dst, data);
}

static void _TCCopyXALTraverserD(void *data, void *param)
{
    _TCCopyX_internal_object_t *io = (_TCCopyX_internal_object_t *)param;

    io->dst->fixed[io->dst->fixedCount++] = io->dataDuplicator(data, io->param);
}

static void _TCCopyXLXTraverserD(void *data, void *param)
{
    _TCCopyX_internal_object_t *io = (_TCCopyX_internal_object_t *)param;

    TCAdd(io->dst, io->dataDuplicator(data, io->param));
}
