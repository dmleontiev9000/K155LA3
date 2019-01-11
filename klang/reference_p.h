#pragma once

#include "reference.h"

namespace K {
namespace Lang {

class Node1;
class Reference1;
class Reference1 : public Reference {
public:
    Reference1();
    Reference1(Node1 *owner, Node1 *target);
    ~Reference1();

    static Reference1 * create(Node1 *owner, Node1 *target);
    Reference1 * next_target() const;
    Reference1 * next_source() const;

    Node1 * get() const { return dst; }
    Node1      *  src;
    Node1      *  dst;
    Reference1 ** src_prev;
    Reference1 *  src_next;
    Reference1 ** dst_prev;
    Reference1 *  dst_next;
    uint          start, end;
private:
    Reference1(const Reference1&) = delete;
    Reference1& operator =(const Reference1&) = delete;
    Reference1(Reference1&&) = delete;
    Reference1& operator =(Reference1&&) = delete;
};

} //namespace Lang
} //namespace K


