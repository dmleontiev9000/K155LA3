#include "lang_p.h"

using namespace K::Lang::Internal;

Reference * Reference::linkSourceAndTarget(Node * src, Node * tgt, unsigned at)
{
    Reference * ptr = ContextPrivate::reference();
    ptr->prev_target = &tgt->incoming_refs;
    ptr->next_target = tgt->incoming_refs;
    tgt->incoming_refs = ptr;
    ptr->prev_source = &src->outgoing_refs;
    ptr->next_source = src->outgoing_refs;
    src->outgoing_refs = ptr;
    ptr->source = src;
    ptr->target = tgt;
    ptr->at     = at;
    ptr->refcount = at == ~0u ? 1:0;
    return ptr;
}
void Reference::dropReferencesSource(Reference * ptr) {
    while(ptr) {
        auto copy = ptr->next_source;
        (*ptr->prev_target) = ptr->next_target;
        (*ptr->prev_source) = ptr->next_source;
        ContextPrivate::destroy(ptr);
        ptr = copy;
    }
}
void Reference::dropReferencesTarget(Reference * ptr) {
    while(ptr) {
        auto copy = ptr->next_target;
        //unlink reference from all lists
        (*ptr->prev_target) = ptr->next_target;
        (*ptr->prev_source) = ptr->next_source;
        ptr->next_source = ptr;
        ptr->prev_source = &ptr->next_source;
        ptr->next_target = ptr;
        ptr->prev_target = &ptr->next_target;
        //invalidate() will not cause invalidation
        //of the references, but will move source to
        //invalidated list.
        ptr->target = nullptr;
        ptr->source->invalidate();
        if (!ptr->refcount) {
            //non-temporary reference has no references from
            //source and can be deleted right now
            ContextPrivate::destroy(ptr);
            //temporary references are owned by parser
            //and will be released during invalidation cycle
        }
        ptr = copy;
    }
}
void Reference::dropTemporaryReference(Reference *ptr) {
    Q_ASSERT(ptr->at == ~0u);
    if (--ptr->refcount == 0) {
        (*ptr->prev_target) = ptr->next_target;
        (*ptr->prev_source) = ptr->next_source;
        ContextPrivate::destroy(ptr);
    }
}
Reference * Reference::reaimReference(Reference *ptr, Node *newtgt) {
    Q_ASSERT(ptr->at == ~0u);
    if (ptr->refcount > 1) {
        --ptr->refcount;
        return linkSourceAndTarget(ptr->source, newtgt);
    } else {
        (*ptr->prev_target) = ptr->next_target;
        ptr->next_target = newtgt->incoming_refs;
        ptr->prev_target = &newtgt->incoming_refs;
        newtgt->incoming_refs = ptr;
        if (ptr->next_target) ptr->next_target->prev_target = &ptr->next_target;
        return ptr;
    }
}
