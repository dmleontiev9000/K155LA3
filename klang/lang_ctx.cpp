#include "lang_p.h"

using namespace ::K::Lang;

static ContextPrivate * bound = nullptr;

ContextPrivate::ContextPrivate() {
    assetpool = new EditUtils::AssetPool();
    nodepool  = new boost::object_pool<PNode>();
    refpool   = new boost::object_pool<PReference>();
}
ContextPrivate::~ContextPrivate() {
    bind();
    //order of deallocation is critical!
    //nodes may try to deallocate some mem from assetpool
    delete nodepool; nodepool = nullptr;
    delete refpool; refpool = nullptr;
    delete assetpool; assetpool = nullptr;
    unbind();
}
void ContextPrivate::bind() {
    Q_ASSERT(bound == nullptr);
    bound = this;
    assetpool->lock();
}
void ContextPrivate::unbind() {
    Q_ASSERT(bound == this);
    assetpool->unlock();
    bound = nullptr;
}
PNode * ContextPrivate::node() {
    return bound->nodepool->construct();
}
PReference * ContextPrivate::reference() {
    return bound->refpool->malloc();
}
void ContextPrivate::destroy(PNode * n) {
    bound->nodepool->destroy(n);
}
void ContextPrivate::destroy(PReference * r) {
    bound->refpool->free(r);
}
PString * ContextPrivate::string(uint nchars) {
    PString * ps = (PString*)bound->assetpool->alloc(sizeof(PString)+nchars*sizeof(PString::Symbol)*nchars);
    ps->string_length = nchars;
    return ps;
}

bool ContextPrivate::process(QElapsedTimer *timer, int max)
{
    /*
     * first, we process invalidation list
     */
    bool q = true;
    if (workset_invalidate)
        q = process_invalidate_workset(timer, max);
    if (!q) return q;
    /*
     * once element is invalidated we try to parse it's text
     */
    if (workset_parse)
        q = process_parse_workset(timer, max);
    if (!q) return q;
    /*
     * parse may complete successfully(element is removed from worksets)
     * placed to blocked list if an element depends on other elements
     * placed to finalize list if an element may be moved to blocked
     * list later: class X { inherits Y; } will successfylly parse "class X"
     * and class object will remain in finalized list, but later child
     * element "inherits Y" will move element to blocked list
     */
    if (workset_blocked)
        q = process_blocked_workset(timer, max);
    if (!q) return q;
    /*
     * if there are blocked elements, but none was moved to finalize and
     * parse lists, blocked elements are considered blocked forever.
     */
    if (workset_blocked && !workset_finalize && !workset_parse)
        return process_totally_blocked(timer, max);
    /*
     * finalized elements may be moved to parse list
     */
    if (workset_finalize)
        q = process_finalize_workset(timer, max);
    return q;
}
/*
 * INVALIDATION
 */
bool ContextPrivate::process_invalidate_workset(QElapsedTimer * timer, int max)
{
    Q_ASSERT(bound == this);
    do {
        //pick a node from invalidate workset
        PNode * node = workset_invalidate;
        if (!node) return true;
        node->detachFromWorkset();
        node->invalidate_data();
        //invalidate subitems
        for(auto i = node->first_chld; i; i = i->next_node)
            i->invalidate();
        node->attachToWorkset(&workset_parse);
    } while(timer->elapsed() < max);
    return false;
}
/*
 * PARSING STAGE
 */
bool ContextPrivate::process_parse_workset(QElapsedTimer *timer, int max)
{
    Q_ASSERT(bound == this);
    do {
        //pick a node from parse workset
        PNode * node = workset_parse;
        if (!node) return true;
        node->detachFromWorkset();

        int q = node->parse(timer, max);
        if (q < 0) {
            node->attachToWorkset(&workset_parse);
            break;
        }
        if (q) {
            node->attachToWorkset(&workset_finalize);
            for(auto i = node->first_chld; i; i = i->next_node)
                i->attachToWorkset(&workset_parse);
        } else {
            node->attachToWorkset(&workset_blocked);
        }
    } while(timer->elapsed() < max);
    return false;
}
/*
 * DEPENDENCY CHECK STAGE
 */
bool ContextPrivate::process_blocked_workset(QElapsedTimer *timer, int max)
{
    bool q = false;
    Q_ASSERT(bound == this);
    do {
        //pick a node from parse workset
        PNode * node = workset_blocked;
        if (!node) { q = true; break; }
        node->detachFromWorkset();

        int q = node->blocked(timer, max);
        if (!q) {
            node->attachToWorkset(&workset_blocked);
            break;
        }
        if (q)
            node->attachToWorkset(&workset_blocked2);
        else
            node->attachToWorkset(&workset_finalize);
    } while(timer->elapsed() < max);
    if (!workset_blocked && workset_blocked2) {
        workset_blocked = workset_blocked2;
        workset_blocked2 = nullptr;
        workset_blocked->prev_link = &workset_blocked;
    }
    return q;
}
/*
 * FINALIZE
 */
bool ContextPrivate::process_finalize_workset(QElapsedTimer *timer, int max)
{
    Q_ASSERT(bound == this);
    do {
        //pick a node from parse workset
        PNode * node = workset_finalize;
        if (!node) return true;
        node->detachFromWorkset();

        int q = node->finalize(timer, max);
        if (q < 0) {
            node->attachToWorkset(&workset_finalize);
            break;
        }
        if (!q)
            node->attachToWorkset(&workset_blocked);
    } while(timer->elapsed() < 50);
    return false;
}
