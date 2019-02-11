#include "node_p.h"
#include "context_p.h"
using namespace K::Lang;

Node1::Node1(ContextPrivate * ctx) {
    mContext= ctx;
    mType   = INVALID|FORCE_DISABLED;
    mQueue  = EvalOrder::STMTS;
    mSMEntry = nullptr;
    mWSPrev = &mWSNext;
    mWSNext = nullptr;
    mNLPrev = &mNLNext;
    mNLNext = nullptr;
    mParent = nullptr;
    mNext   = this;
    mPrev   = this;
    mChild  = nullptr;
    mOut    = nullptr;
    mIn     = nullptr;
    mDeclType = nullptr;
}
Node1::~Node1() {
    if (mIn) mIn->invalidate();
    if (mOut) mOut->destroyList();
    if (mSMEntry)
        mContext->strmap->removeNode(this);
    Q_ASSERT(mChild == nullptr);
    Q_ASSERT(mWSPrev == &mWSNext);
    Q_ASSERT(mWSNext == nullptr);
    Q_ASSERT(mSMEntry == nullptr);
    Q_ASSERT(mNLPrev == &mNLNext);
    Q_ASSERT(mNLNext == nullptr);
}
void   Node1::setName(uint start, uint end) {    
    Q_ASSERT((bool)mText);
    Q_ASSERT(start < end);
    mContext->strmap->addNode(this, start, end);
}
void   Node1::unsetName() {
    if (mSMEntry)
        mContext->strmap->removeNode(this);
}
Node1 *Node1::attachToWorkset(Node1 **workset) {
    auto n = mWSNext;
    *mWSPrev = mWSNext;
    if (mWSNext) mWSNext->mWSPrev = mWSPrev;

    mWSPrev = workset;
    mWSNext = *workset;
    if (mWSNext) mWSNext->mWSPrev = &mWSNext;
    *workset = this;
    return n;
}
Node1 *Node1::detachFromWorkset() {
    auto n = mWSNext;
    *mWSPrev = mWSNext;
    if (mWSNext) mWSNext->mWSPrev = mWSPrev;
    mWSPrev = &mWSNext;
    mWSNext = nullptr;
    return n;
}
Node1* Node1::next() const {
    if (Q_UNLIKELY(!mParent))
        return nullptr;
    if (mNext == mParent->mChild)
        return nullptr;
    return mNext;
}
Node1* Node1::prev() const {
    if (Q_UNLIKELY(!mParent))
        return nullptr;
    if (this == mParent->mChild)
        return nullptr;
    return mPrev;
}
Node * Node::create(Context *ctx) {
    Node1 * out = new(ctx->d->nodepool->malloc()) Node1(ctx->d);
    return out;
}
Node * Node::create(Context *ctx, const char * text) {
    uint len = strlen(text);
    auto str = String::alloc(ctx->d->assetpool, len);
    if (!str) return nullptr;
    for(uint n = 0; n < len; ++n)
        str->set(n, QChar::fromLatin1(text[n]));

    Node1 * out = new(ctx->d->nodepool->malloc()) Node1(ctx->d);
    out->mText = str;
    return out;
}
Node * Node::create(Context *ctx, const QStringRef &text) {
    uint len = text.length();
    auto str = String::alloc(ctx->d->assetpool, len);
    if (!str) return nullptr;
    for(uint n = 0; n < len; ++n)
        str->set(n, text[n].unicode());

    Node1 * out = new(ctx->d->nodepool->malloc()) Node1(ctx->d);
    out->mText = str;
    return out;
}
Node * Node::create(Context *ctx, const QString &text) {
    uint len = text.length();
    auto str = String::alloc(ctx->d->assetpool, len);
    if (!str) return nullptr;
    for(uint n = 0; n < len; ++n)
        str->set(n, text[n].unicode());

    Node1 * out = new(ctx->d->nodepool->malloc()) Node1(ctx->d);
    out->mText = str;
    return out;
}
void   Node::attach(Node * __restrict__ attachment, Node * __restrict__ after) {
    Node1 * __restrict__ self = static_cast<Node1*>(this);
    Node1 * __restrict__ at   = static_cast<Node1*>(attachment);
    Node1 * __restrict__ af   = nullptr;
    Q_ASSERT(at->mParent == nullptr);
    Q_ASSERT(at->mContext == self->mContext);
    Q_ASSERT(at->mPrev == at);
    Q_ASSERT(at->mNext == at);
    at->mParent = self;

    if (after) {
        af = static_cast<Node1*>(after);
        Q_ASSERT(af->mParent == self);
        Q_ASSERT(af->mContext == self->mContext);

        at->mPrev = at;
        at->mNext = af->mNext;
        at->mPrev->mNext = at;
        af->mNext->mPrev = at;
    } else if (self->mChild) {
        at->mPrev = self->mChild->mPrev;
        at->mNext = self->mChild;
        at->mPrev->mNext = at;
        af->mNext->mPrev = at;
        self->mChild = at;
    } else {
        self->mChild = at;
    }
    invalidate();
}
void   Node::detach() {
    Node1 * __restrict__ self = static_cast<Node1*>(this);
    if (!self->mParent) {
        Q_ASSERT(self->mPrev == self);
        Q_ASSERT(self->mNext == self);
    } else {
        Node1 * nc = self->mNext == self ? nullptr : self->mNext;
        self->mPrev->mNext = self->mNext;
        self->mNext->mPrev = self->mPrev;
        if (self->mParent->mChild == self)
            self->mParent->mChild = nc;
        self->mParent->invalidate();
        self->disable();
        self->mParent = nullptr;
    }
}
void   Node::destroy() {
    Node1 * __restrict__ self = static_cast<Node1*>(this);
    self->mType = INVALID;
    self->unsetName();
    detach();
    self->attachToWorkset(&self->mContext->workset_finalize);
}
void   Node::invalidate() {
    Node1 * __restrict__ self = static_cast<Node1*>(this);
    self->mType = INVALID;
    self->unsetName();
    self->attachToWorkset(&self->mContext->workset_invalidate);
}
void   Node::recheck() {
    Node1 * __restrict__ self = static_cast<Node1*>(this);
    self->attachToWorkset(&self->mContext->workset_recheck);
}
void   Node::comment(bool c) {
    (static_cast<Node1*>(this))->comment(c);
}
bool   Node::comment() const {
    return (static_cast<const Node1*>(this))->comment();
}
void   Node::enable() {
    (static_cast<Node1*>(this))->enable();
}
void   Node::disable() {
    (static_cast<Node1*>(this))->disable();
}
bool   Node::disabled() const {
    return (static_cast<const Node1*>(this))->disabled();
}
uint   Node::type() const {
    return (static_cast<const Node1*>(this))->mType;
}
bool   Node::isValid() const {
    return (static_cast<const Node1*>(this))->mType != INVALID;
}
Node * Node::parent() const {
    return (static_cast<const Node1*>(this))->mParent;
}
Node * Node::next() const {
    return (static_cast<const Node1*>(this))->next();
}
Node * Node::previous() const {
    return (static_cast<const Node1*>(this))->prev();
}
Node * Node::firstChild() const {
    return (static_cast<const Node1*>(this))->mChild;
}
Node * Node::lastChild() const {
    auto c = (static_cast<const Node1*>(this))->mChild;
    if (c) c = c->mPrev;
    return c;
}
Node * Node::firstExplicitChild() const {
    const Node1 * __restrict__ self = static_cast<const Node1*>(this);
    auto c0 = self->mChild;
    if (!c0) return nullptr;

    constexpr uint m =
            (1<<INSTANCE)|
            (1<<TPARAMETER)|
            (1<<VPARAMETER)|
            (1<<ARGUMENT)|
            (1<<ATTRIBUTE);
    auto c = c0;
    do {
        if (((1<<c->mType) & m) == 0)
            return c;
        c = c->mNext;
    } while(c!=c0);
    return nullptr;
}
Node * Node::declType() const {
    const Node1 * __restrict__ self = static_cast<const Node1*>(this);
    if (!self->mDeclType)
        return nullptr;
    return self->mDeclType->dst;
}
