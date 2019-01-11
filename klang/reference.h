#pragma once

#include "klang_global.h"

namespace K {
namespace Lang {

class Node;
class Reference;
class K_LANG_EXPORT Reference {
public:
    void invalidate();
    void destroy();
    void operator =(Node * node);
    Node * get() const;
    uint start() const;
    uint end() const;
protected:
    Reference() {}
    ~Reference();
};

} //namespace Lang;
} //namespace K;

