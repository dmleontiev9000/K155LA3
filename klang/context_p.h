#pragma once

#include "klang_global.h"
#include "context.h"
#include "node_p.h"
#include "reference_p.h"
#include "ast.h"
#include "strmap.h"
#include <assetpool.h>
#include <boost/pool/object_pool.hpp>
#include <QObject>
#include <QVector>
#include <QMultiHash>
#include <QVariant>
#include <QEventLoop>

#define NUM_QUEUES static_cast<int>(EvalOrder::MAX_EVAL_ORDER)
namespace K {
namespace Lang {

class ContextPrivate {
public:
    ContextPrivate(ASTGenerator * gen);
    ~ContextPrivate();

    bool                             mustbedeleted;

    boost::object_pool<Node1>      * nodepool;
    boost::object_pool<Reference1> * refpool;
    K::EditUtils::AssetPool        * assetpool;
    ASTGenerator                   * generator;
    StringMap                      * strmap;

    //deletion queue
    Node1                          * workset_finalize;
    //invalidation queue
    Node1                          * workset_invalidate;
    //determine name of node if possible without wasting resources
    Node1                          * workset_preeval;
    //processing queues
    //multiple queues are used to minimize number of analyzed elements
    //which are delayed by their dependencies. if elements needs an
    //element which is not processed,
    Node1                          * workset_queue[NUM_QUEUES];
    Node1                          * workset_waiting[NUM_QUEUES];
    Node1                          * workset_blocked[NUM_QUEUES];
    //complete queue
    Node1                          * workset_complete;

    static Node       * node();
    static void         destroy(Node *);
    static Reference *  reference(Node *from, Node *to);
    static void         destroy(Reference *);
    static String    *  string(uint nchars);
    
    bool process();

    bool process_finalize_workset(QEventLoop * loop);
    bool process_invalidate_workset(QEventLoop * loop);
    bool process_preevaluate_workset(QEventLoop * loop);
    bool process_queue(QEventLoop * loop);
    bool process_blocked_workset(QEventLoop * loop);
    bool process_totally_blocked(QEventLoop * loop);
};

} //namespace Lang
} //namespace K

