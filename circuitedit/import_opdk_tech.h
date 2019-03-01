#include "import_opdk_misc.h"

/*******************************************

OPS_OPS
 |
 +--OPS_Technology
 |   |
 |   +--OPS_TechnologySettingsDefault;
 |   |   |
 |   |   +--OPC_DesignGrid;
 |   |   |
 |   |   +--OPS_SIUnit
 |   |
 |   +--OPS_MetalStack
 |   |   |
 |   |   +--OPS_ConnectivityTable
 |   |       |
 |   |       +--OPS_ConnectObject[]
 |   |
 |   +--OPS_GenericLayers
 |   |   |
 |   |   +--OPS_GenericLayer[]
 |   |       |
 |   |       +--OPS_GenericLayerElement[]
 |   |
 |   +--OPC_Documentation
 |   |
 |   +--OPC_Reference
 |
 ....

 *******************************************/

namespace K {
namespace CE {
namespace OPDK {
namespace Import {

class OPS_TechnologySettingsDefault : public Base {
public:
    OPS_TechnologySettingsDefault()
        : Base(ops, "TechnologySettingsDefault")
        , grid("DesignGrid", OPS_Value::Positive)
        , siunit(SI_Unit::Meter)
    {}
    void clear();
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    bool endElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    bool             valid;
    OPS_Value        grid;
    SI_Unit          siunit;
};

class OPS_Option : public Base {
public:
    OPS_Option() : Base(ops, "Option") {}
    void clear();
    bool params(QXmlStreamReader *reader, QStringList *messageLog) override;
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    bool endElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    QStringList                   names;
    QStringList                   docs;
    QStringList                   refs;
    Documentation                 documentation;
    DocReference                  reference;
};

class OPS_GenericLayerElement : public Base {
public:
    OPS_GenericLayerElement() : Base(ops, "GenericLayerElement")
      , actual("ActualLayer"), reference("ReferenceLayer") {}
    void clear();
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    bool endElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    bool         valid;
    QStringList  actual_list;
    QStringList  reference_list;
    OPS_RefLayer actual;
    OPS_RefLayer reference;
};

class OPS_GenericLayer : public Base {
public:
    OPS_GenericLayer() : Base(ops, "GenericLayer") {}
    struct Info {
        uint                    type;
        QString                 name;
        QStringList             actual;
        QStringList             refs;
    };
    void clear();
    bool params(QXmlStreamReader *reader, QStringList *messageLog) override;
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    bool endElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    QList<Info>           layers;
    OPS_GenericLayerElement element;
};

class OPS_GenericLayers : public Base {
public:
    OPS_GenericLayers() : Base(ops, "GenericLayers") {}
    void clear();
    const QList<OPS_GenericLayer::Info>& info() const;
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override ;
    OPS_GenericLayer layer;
};

class OPS_ConnectObject : public Base {
public:
    enum class Type { Touch, Via, Substrate };
    OPS_ConnectObject() : Base(ops, "ConnectObject")
      , fromLayer("FromLayer")
      , toLayer("ToLayer")
      , viaLayer("ThroughLayer")
    {}

    void clear();
    bool params(QXmlStreamReader *reader, QStringList *messageLog) override;
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    bool endElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    struct Entry {
        Type    type;
        int     ordering;
        QString fromLayer;
        QString toLayer;
        QString viaLayer;
    };
    OPS_RefLayer fromLayer;
    OPS_RefLayer toLayer;
    OPS_RefLayer viaLayer;
    QList<Entry> entries;
};

class OPS_ConnectivityTable : public Base {
public:
    OPS_ConnectivityTable() : Base(ops, "ConnectivityTable") {}
    void clear();
    QList<OPS_ConnectObject::Entry>&& entries();
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    OPS_ConnectObject connobj;
};

class OPS_MetalStack : public Base {
public:
    OPS_MetalStack() : Base(ops, "MetalStack") {}
    void clear();
    bool valid();
    bool params(QXmlStreamReader *reader, QStringList *messageLog) override;
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    bool endElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    struct MetalStack {
        QString name;
        QList<OPS_ConnectObject::Entry> connections;
    };
    QList<MetalStack> stacks;
    OPS_ConnectivityTable         conntable;
    Documentation                 documentation;
    DocReference                  reference;
};

class OPS_Technology : public Base {
public:
    OPS_Technology() : Base(ops, "Technology") {}
    void clear();
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    bool endElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    bool                          valid;
    OPS_TechnologySettingsDefault settings_def;
    OPS_MetalStack                metal_stack;
    OPS_Option                    options;
    Documentation                 documentation;
    DocReference                  reference;
};

} //namespace Import
} //namespace SI2
} //namespace CE
} //namespace K
