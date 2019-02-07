#include "reference_p.h"
#include "node_p.h"
#include "context_p.h"
#include <boost/pool/object_pool.hpp>

using namespace K::Lang;

Reference1::Reference1() {
    src = dst = nullptr;
    //list ends point to self
    src_prev = &src_next;
    dst_prev = &dst_next;
    src_next = dst_next = nullptr;
    start = end = 0;
}
Reference1::Reference1(Node1 * owner, Node1 * target) {
    Q_ASSERT(owner != nullptr);
    src      = owner;
    src_prev = &owner->mOut;
    src_next = owner->mOut;
    if (owner->mOut)
        owner->mOut->src_prev = &dst_next;
    owner->mOut = this;

    dst_prev = &dst_next;
    dst_next = nullptr;
    start = end = 0;
    if (target) {
        dst      = target;
        dst_prev = &target->mIn;
        dst_next = target->mIn;
        if (target->mIn)
            target->mIn->dst_prev = &dst_next;
        target->mIn = this;
    }
}
Reference1::~Reference1() {
    if (src_next)
        src_next->src_prev = src_prev;
    if (dst_next)
        dst_next->dst_prev = dst_prev;
    *src_prev = src_next;
    *dst_prev = dst_next;
}
Reference1 * Reference1::create(Node1 *owner, Node1 *target) {
    Q_ASSERT(owner != nullptr);
    Q_ASSERT(target != nullptr);
    return new(owner->mContext->refpool->malloc()) Reference1(owner,target);
}
Reference1 * Reference1::next_target() const {
    return dst_next == this ? nullptr : dst_next;
}
Reference1 * Reference1::next_source() const {
    return src_next == this ? nullptr : src_next;
}
void Reference::operator =(Node * target) {
    Reference1 * __restrict__ self = static_cast<Reference1*>(this);
    if (self->dst_next)
        self->dst_next->dst_prev = self->dst_prev;
    self->dst_prev = &self->dst_next;
    self->dst_next = nullptr;
    self->dst      = nullptr;
    if (target) {
        auto t = static_cast<Node1*>(target);
        self->dst = t;
        self->dst_prev = &t->mIn;
        self->dst_next = t->mIn;
        if (t->mIn)
            t->mIn->dst_prev = &self->dst_next;
        t->mIn = self;
    }
}
void   Reference::destroy() {
    auto self = static_cast<Reference1*>(this);
    auto s = self->src;
    self->~Reference1();
    s->mContext->refpool->free(self);
}
void   Reference::destroyList() {
    auto self = static_cast<Reference1*>(this);
    do {
        if (self->dst_next)
            self->dst_next->dst_prev = self->dst_prev;
        *self->dst_prev = self->dst_next;
        auto next = self->src_next;
        self->src->mContext->refpool->free(self);
        self = next;
    } while(self);
}
void   Reference::invalidate() {
    Reference1 * __restrict__ self = static_cast<Reference1*>(this);
    do {
        self->src->invalidate();
        auto next = self->dst_next;
        self->dst_prev = &self->dst_next;
        self->dst_next = nullptr;
        self->dst      = nullptr;
        self = next;
    } while(self);
}
Node * Reference::get() const {
    return (static_cast<const Reference1*>(this))->dst;
}
uint Reference::start() const {
    return (static_cast<const Reference1*>(this))->start;
}
uint Reference::end() const {
    return (static_cast<const Reference1*>(this))->end;
}
