#include "assetpool.h"
#include <malloc.h>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <string.h>

#define MEMORY_BARRIER {asm volatile("": : :"memory");}

using namespace K::EditUtils;

AssetPool::AssetPool()
{
    SliceHdr * sh = (SliceHdr*)aligned_malloc(SLICE, SLICE);
    if (!sh) throw std::bad_alloc();
    sh->gc_next   = nullptr;
    sh->alloc     = AL;

    gc_stop       = false;
    pool_head     = sh;
    pool_tail     = sh;
    iteration_count = 0;

    start();
}
AssetPool::~AssetPool()
{
    gc_stop = true;
    wait();

    SliceHdr * h = pool_head;
    do {
        auto n = h->gc_next;
        aligned_free(h);
        h = (SliceHdr*)n;
    } while(h);
    pool_head = nullptr;
}

Element * AssetPool::alloc(unsigned length)
{
    Q_ASSERT(length >= sizeof(Element));
    Q_ASSERT(length <= SLICE-AL);

    Element * q;

    length = guessSize(length);

    if (Q_UNLIKELY(pool_tail->alloc + length > SLICE)) {
        SliceHdr * sh = (SliceHdr*)aligned_malloc(SLICE, SLICE);
        if (!sh)
            throw std::bad_alloc();
        sh->alloc   = AL + length;
        sh->gc_next = nullptr;

        char * __restrict address = (char*)sh;
        address += AL;
        memset(address, 0, length);
        q = (Element*)address;
        q->magic  = LIVEMAGIC;
        q->memsize = length;
        //memory barrier before we make this slice visible to GC...
        MEMORY_BARRIER;
        pool_tail->gc_next = sh;
        pool_tail = sh;
    } else {
        SliceHdr * sh = pool_tail;
        char * __restrict address = (char*)sh;
        address += sh->alloc;
        memset(address, 0, length);
        q = (Element*)address;
        q->magic  = LIVEMAGIC;
        q->memsize = length;
        //memory barrier before we make this allocation visible to GC...
        MEMORY_BARRIER;
        sh->alloc += length;
    }
    return q;
}

void AssetPool::run() {
    SliceHdr * sh = pool_head;
    SliceHdr * ph = nullptr;
    while(!gc_stop)
    {
        //do not touch current slice! it is used by allocator
        if (!sh->gc_next) {
            QThread::sleep(1);
            sh  = pool_head;
            ph  = nullptr;
            continue;
        }

        /*
         * исследуем отрезок хипа
         */
        unsigned p = AL;
        unsigned alloc = sh->alloc;
        unsigned usage = AL;
        Q_ASSERT(alloc <= SLICE);
        Q_ASSERT(alloc >= AL);

        Element * __restrict holes[16];
        Element * __restrict pe = nullptr;
        unsigned  last_hole = 0;
        int       nholes    = 0;
        int       holeidx   = 0;

        while(p < alloc && nholes < 16)
        {
            Element * __restrict e = (Element*)(((char*)sh)+p);
            if (e->magic == LIVEMAGIC) {
                usage += e->memsize;
                p     += e->memsize;
            } else if (p == last_hole) {
                Q_ASSERT(e->magic == DEADMAGIC);
                pe->memsize += e->memsize;
                p          += e->memsize;
                last_hole   = p;
            } else {
                Q_ASSERT(e->magic == DEADMAGIC);
                pe          = e;
                p          += e->memsize;
                last_hole   = p;

                //record all holes
                holes[holeidx++] = e;
                holeidx &= 15;
                nholes++;
            }
        }
        Q_ASSERT(p == alloc);
        Q_ASSERT(usage <= alloc);

        if (usage == AL) {
            void * unused_block = sh;

            if (ph) {
                ph->gc_next = sh->gc_next;
            } else {
                pool_head = (SliceHdr*)sh->gc_next;
            }
            sh = (SliceHdr*)sh->gc_next;

            aligned_free(unused_block);
            continue;
        }
        //if more than 3/4 of allocated size is used, don't compact:
        //we can afford wasting 25% of heap for speed considerations
        if (usage > 3*SLICE/4)
        {
            ph = sh;
            sh = (SliceHdr*)sh->gc_next;
            continue;
        }

        /*critical part*/{
            QMutexLocker locker(&mutex);
            ++iteration_count;
            //if more than half of segment is used, compact it
            //holes will usually spawn at limited rate and we will
            //will fit into 16 holes limit in most cases
            if (2u*alloc > SLICE && nholes) {
                char * dst  = (char*)holes[0];
                char * src  = dst + holes[0]->memsize;

                for(int i = 1; i < nholes; ++i) {
                    char * end = (char*)holes[i];
                    if (src < end) {
                        relocate(dst, src, end);
                        memmove(dst, src, end-src);
                    }
                    dst += end-src;
                    src  = (char*)holes[i] + holes[i]->memsize;
                }
                char * end = (char*)sh + sh->alloc;
                if (src < end) {
                    relocate(dst, src, end);
                    memmove(dst, src, end-src);
                }
                dst += end-src;

                sh->alloc = dst-(char*)sh;
                if (sh->alloc*2 < SLICE) {
                    ph = (SliceHdr*)sh;
                    sh = (SliceHdr*)sh->gc_next;
                }
                continue;
            }
            if (ph && ph->alloc+alloc <= SLICE) {

                char * src = ((char*)sh)+AL;
                char * dst = ((char*)ph)+ph->alloc;
                char * end = ((char*)sh)+sh->alloc;

                relocate(dst, src, end);
                auto len = end-src;
                memmove(dst, src, len);
                ph->alloc += len;

                void * unused_block = sh;

                ph->gc_next = sh->gc_next;
                sh = (SliceHdr*)sh->gc_next;

                aligned_free(unused_block);
                continue;
            }
        }
        ph = sh;
        sh = (SliceHdr*)sh->gc_next;
    }
}

void AssetPool::relocate(char *dst, char *src, char *end)
{
    char  * p = src;
    int delta = src-dst;
    while(p < end)
    {
        Element * e = (Element*)p;
        if (e->magic == LIVEMAGIC) {
            if (e->handle) *e->handle = (Element*)(p - delta);
        }
        p += e->memsize;
    }
    Q_ASSERT(p==end);
}
void Element::dispose()
{
    MEMORY_BARRIER;
    magic  = AssetPool::DEADMAGIC;
    handle = 0;
    MEMORY_BARRIER;
}
