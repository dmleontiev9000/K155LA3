#include "context_p.h"
#include <QEventLoop>

using namespace ::K::Lang;

Context::Context(QObject *parent)
    : QObject(parent)
    , d(new ContextPrivate(xxx))
{

}
Context::~Context()
{
    delete d;
}
void Context::deleteLater() {
    if (!d->mustbedeleted) {
        d->mustbedeleted = true;
        deleteLater();
    }
}
ContextPrivate::ContextPrivate(ASTGenerator * gen) {
    mustbedeleted = false;

    assetpool = new EditUtils::AssetPool();
    nodepool  = new boost::object_pool<Node1>();
    refpool   = new boost::object_pool<Reference1>();
    strmap    = new StringMap;
    generator = gen;
    
    workset_finalize   = nullptr;
    workset_invalidate = nullptr;
    workset_recheck    = nullptr;
    for(int i = 0; i < NUM_QUEUES; ++i) {
        workset_queue[i]   = nullptr;
        workset_waiting[i] = nullptr;
        workset_blocked[i] = nullptr;
    }
    workset_complete   = nullptr;
}
ContextPrivate::~ContextPrivate() {
    //order of deallocation is critical!
    //nodes may try to deallocate some mem from assetpool
    delete nodepool; nodepool = nullptr;
    delete refpool; refpool = nullptr;
    delete assetpool; assetpool = nullptr;
    delete strmap;
}


