#pragma once

#include "klang_global.h"
#include "lang.h"
#include "str.h"
#include <assetpool.h>
#include <boost/pool/object_pool.hpp>
#include <QObject>
#include <QVector>
#include <QMultiHash>
#include <QElapsedTimer>
#include <QVariant>

namespace K {
namespace Lang {
namespace Internal {

struct Node;
struct Reference;
struct Metadata;
struct Tokenizer;
struct Node {//~128 bytes
    Node(){}
    ~Node();
    ContextPrivate * context;
    quint32     flags = 0;
    quint32     namestart = 0, nameend = 0;
    //parent-chil
    String    * text        = nullptr;
    Tokenizer* parserdata  = nullptr;
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
    int  parse(QElapsedTimer * timer, int max);
    int  blocked(QElapsedTimer * timer, int max);
    int  finalize(QElapsedTimer * timer, int max);

    void attachToWorkset(Node ** set);
    void detachFromWorkset();
};

struct Reference {
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

} //namespace Internal

typedef Internal::Node PNode;
typedef Internal::Reference PReference;
typedef Internal::String PString;

class ContextPrivate {
public:
    ContextPrivate();
    ~ContextPrivate();

    boost::object_pool<PNode>      * nodepool;
    boost::object_pool<PReference> * refpool;
    K::EditUtils::AssetPool        * assetpool;
    PNode                          * workset_invalidate = nullptr;
    PNode                          * workset_parse      = nullptr;
    PNode                          * workset_blocked    = nullptr;
    PNode                          * workset_blocked2   = nullptr;
    PNode                          * workset_finalize   = nullptr;
    int                              event              = -1;

    static PNode      * node();
    static void         destroy(PNode *);
    static PReference * reference();
    static void         destroy(PReference *);
    static PString    * string(uint nchars);

    void bind();
    void unbind();
    bool process(QElapsedTimer * timer, int max);
    bool process_invalidate_workset(QElapsedTimer * timer, int max);
    bool process_parse_workset(QElapsedTimer * timer, int max);
    bool process_type_workset(QElapsedTimer * timer, int max);
    bool process_blocked_workset(QElapsedTimer * timer, int max);
    bool process_finalize_workset(QElapsedTimer * timer, int max);
    bool process_totally_blocked(QElapsedTimer * timer, int max);
};

} //namespace Lang
} //namespace K


