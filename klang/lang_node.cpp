#include "lang_p.h"

using namespace K::Lang::Internal;

void Node::invalidate() {
    flags = 0;
    attachToWorkset(&context->workset_invalidate);
}
int  Node::invalidate_data() {
    namestart = nameend = 0;

    if (outgoing_refs) {
        Reference::dropReferencesSource(outgoing_refs);
        outgoing_refs = nullptr;
    }
    if (incoming_refs) {
        Reference::dropReferencesTarget(incoming_refs);
        incoming_refs = nullptr;
    }
    delete parserdata;
    delete metadata;
    parserdata = nullptr;
    metadata   = nullptr;
}
void Node::attachToWorkset(Node **set) {
    //unlink from workset
    *prev_link = next_link;
    if (next_link) next_link->prev_link = prev_link;
    //link to invalidation workset
    next_link  = *set;
    *set       = this;
    prev_link  = set;
    if (next_link) next_link->prev_link = &next_link;
}
void Node::detachFromWorkset() {
    //unlink from workset
    *prev_link = next_link;
    if (next_link) next_link->prev_link = prev_link;
    next_link = nullptr;
    prev_link = &next_link;
}
