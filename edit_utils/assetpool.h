#pragma once

#include "editutils_global.h"
#include "../core/compat.h"
#include <QThread>
#include <QMutex>

namespace K {
namespace EditUtils {

struct EDITUTILSSHARED_EXPORT Element
{
    std::uint32_t magic;
    std::uint32_t length;
    Element     **handle;
    void dispose();
};
class EDITUTILSSHARED_EXPORT AssetPool : public QThread {
public:
    enum : unsigned {
        LIVEMAGIC = 0x34353637,
        DEADMAGIC = 0xDEADDEAD,
        SLICE     = 16*1024,
        AL        = 32,
    };

    AssetPool();
    ~AssetPool();

    Element * alloc(unsigned length);
    inline unsigned guessSize(unsigned s) const {
        return (s+AL-1u) &~ (AL-1u);
    }
    quint64 iterationCount() { return iterationCount(); }
    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }
private:
    Q_DISABLE_COPY(AssetPool)
    void run() override;

    struct SliceHdr {
        volatile std::uint32_t alloc;
        volatile SliceHdr    * gc_next;
    };

    QMutex                mutex;
    quint64               iteration_count;
    volatile bool         gc_stop;
    SliceHdr            * pool_head;
    SliceHdr            * pool_tail;

    void compact();
    void relocate(char * dst, char * src, char * end);
    void split(int row, int at);
    void concat(int row);

};


}//namespace EditUtils;
}//namespace K

