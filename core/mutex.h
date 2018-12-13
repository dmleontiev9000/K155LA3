#pragma once

#include "core_global.h"
#include "../core/compat.h"
#include <cstdint>
#include <QAtomicInteger>

namespace K {

class K_CORE_EXPORT Mutex
{
public:
    typedef quint32 DataType;
    Mutex(DataType value = 0) : mValue(value) {}
    ~Mutex() {}

    DataType tryLock(DataType set,      //bits to set
                     DataType require,  //bits which must be set
                     DataType avoid)    //bits which must not be set
    { return tryLock(&mValue, set, require, avoid); }

    DataType lock   (DataType set,      //bits to set
                     DataType require,  //bits which must be set
                     DataType avoid,    //bits which must not be set
                     DataType nowait)   //bits which interrupt wait immediately
    { return lock(&mValue, set, require, avoid, nowait); }

    DataType unlock (DataType were_set) //bits to unset
    { return unlock(&mValue, were_set); }

    static
    DataType tryLock(DataType * ptr,
                     DataType set,      //bits to set
                     DataType require,  //bits which must be set
                     DataType avoid);   //bits which must not be set
    static
    DataType lock   (DataType * ptr,
                     DataType set,      //bits to set
                     DataType require,  //bits which must be set
                     DataType avoid,    //bits which must not be set
                     DataType nowait);  //bits which interrupt wait immediately
    static
    DataType unlock (DataType * ptr,
                     DataType were_set);//bits to unset
private:
    DataType mValue;
};


} //namespace K
