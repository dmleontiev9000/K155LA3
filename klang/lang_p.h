#pragma once

#include "klang_global.h"
#include "lang.h"
#include "str.h"
#include <assetpool.h>
#include <boost/pool/object_pool.hpp>
#include <QObject>
#include <QVector>
#include <QMultiHash>
#include <QVariant>

namespace K {
namespace Lang {

struct Node;
struct Reference;
struct Metadata;

class Node {//~128 bytes
public:
    Node(){}
    ~Node();
    ContextPrivate * context;
    quint32     flags = 0;
    quint32     namestart = 0, nameend = 0;
    //parent-chil
    String    * text        = nullptr;
    Tokenizer * parserdata  = nullptr;
    Metadata  * metadata    = nullptr;
    //children linkage
    Node      * parent;
    Node      * prev_node;
    Node      * next_node;
    Node      * first_chld;
    Node      * last_chld;
    //linkage information
    Reference * outgoing_refs;
    Reference * incoming_refs;
    //update information
    Node     ** prev_link;
    Node      * next_link;

    void attach(Node * at, Node * before);
    void detach();

    void invalidate();
    void invalidate_data();
    int  parse(const InterruptTest&);
    int  blocked(const InterruptTest&);
    int  finalize(const InterruptTest&);

    void attachToWorkset(Node ** set);
    void detachFromWorkset();
};

class Reference {
public:
    Reference   ** prev_target;
    Reference    * next_target;
    Reference   ** prev_source;
    Reference    * next_source;
    Node         * target;
    Node         * source;
    unsigned       at, refcount;

    static Reference * linkSourceAndTarget(Node * src, Node * tgt, unsigned at = ~0u);
    static void dropReferencesSource(Reference * ptr);
    static void dropReferencesTarget(Reference * ptr);
    static void dropTemporaryReference(Reference * ptr);
    static Reference *reaimReference(Reference * ptr, Node * newtgt);
};

struct K_LANG_EXPORT Metadata {
    void unref();
};

class ContextPrivate {
public:
    ContextPrivate();
    ~ContextPrivate();

    int                              event              = -1;
    boost::object_pool<Node>       * nodepool;
    boost::object_pool<Reference>  * refpool;
    K::EditUtils::AssetPool        * assetpool;
    Node                           * workset_invalidate = nullptr;
    Node                           * workset_parse      = nullptr;
    Node                           * workset_blocked    = nullptr;
    Node                           * workset_blocked2   = nullptr;
    Node                           * workset_finalize   = nullptr;
    QMultiHash<Node, Node>           dep_map;

    static Node       * node();
    static void         destroy(Node *);
    static Reference *  reference();
    static void         destroy(Reference *);
    static String    *  string(uint nchars);

    void bind();
    void unbind();
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

