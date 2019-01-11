#pragma once

#include "node.h"

namespace K {
namespace Lang {

class Node1;
class Reference1;
class ContextPrivate;
class Node1 : public Node {
public:
    Node1(ContextPrivate * ctx);
    ~Node1();

    void   destroy();
    void   invalidate();
    ContextPrivate * mContext;

    bool   comment() const {
        return (mType & FORCE_COMMENT)!=0;
    }
    void   comment(bool c) {
        mType ^= ((uint(c)-1) & FORCE_COMMENT) ^ FORCE_COMMENT;
        invalidate();
    }
    bool   disabled() const {
        return (mType & FORCE_DISABLED)!=0;
    }
    void   enable() {
        mType &= ~FORCE_DISABLED;
        invalidate();
    }
    void   disable() {
        mType |= FORCE_DISABLED;
        invalidate();
    }
    void   setdisabled(bool d) {
        mType ^= ((uint(d)-1) & FORCE_DISABLED) ^ FORCE_DISABLED;
        invalidate();
    }
    uint         mType;
    String     * mText;
    uint         mNameStart, mNameEnd;

    void   attachToWorkset(Node1 ** workset);
    void   detachFromWorkset();
    Node1**mWSPrev, *mWSNext;

    Node1 * next() const;
    Node1 * prev() const;
    Node1 * mParent, * mNext, * mPrev;
    Node1 * mChild;

    Reference1 * mOut, * mIn;
    Reference1 * mDeclType;
};

} //namespace Lang
} //namespace K

