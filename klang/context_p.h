#pragma once

#include "klang_global.h"
#include "context.h"
#include "node_p.h"
#include "reference_p.h"
#include <assetpool.h>
#include <boost/pool/object_pool.hpp>
#include <QObject>
#include <QVector>
#include <QMultiHash>
#include <QVariant>

namespace K {
namespace Lang {

class ContextPrivate {
public:
    ContextPrivate();
    ~ContextPrivate();

    bool                             running = false;
    boost::object_pool<Node1>      * nodepool;
    boost::object_pool<Reference1> * refpool;
    K::EditUtils::AssetPool        * assetpool;
    Node1                          * workset_invalidate = nullptr;
    //multiple queues are used to minimize number of analyzed elements
    //which are blocked by dependency chain
    enum {
        QUEUE_NAMESPACE_OR_COMMENT,
        QUEUE_ENUM,
        QUEUE_CONST,
        QUEUE_CLASS,
        QUEUE_VARIABLE,
        QUEUE_FUNCTION,
        QUEUE_BLOCK,
        NUM_QUEUES
    };
    Node1                          * workset_queue[NUM_QUEUES];
    Node1                          * workset_blocked    = nullptr;
    Node1                          * workset_blocked2   = nullptr;
    Node1                          * workset_complete   = nullptr;
    Node1                          * workset_finalize   = nullptr;

    static Node       * node();
    static void         destroy(Node *);
    static Reference *  reference(Node *from, Node *to);
    static void         destroy(Reference *);
    static String    *  string(uint nchars);

    bool process(const InterruptTest& itest);
    bool process_invalidate_workset(const InterruptTest& itest);
    bool process_parse_workset(const InterruptTest& itest);
    bool process_type_workset(const InterruptTest& itest);
    bool process_blocked_workset(const InterruptTest& itest);
    bool process_finalize_workset(const InterruptTest& itest);
    bool process_totally_blocked(const InterruptTest& itest);
};

} //namespace Lang
} //namespace K

