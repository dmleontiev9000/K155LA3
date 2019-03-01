#pragma once
#include "import_opdk.h"
#include <QPen>
#include <QBrush>
namespace K {
namespace CE {
namespace OPDK {
namespace Import {

class OPS_CADLayer : public Base {
public:
    OPS_CADLayer() : Base(ops, "CADLayer") {}
    enum {NONE,LOCKED,UNLOCKED};
    struct Info {
        int     role;
        int     exponumber;
        QString name;
        QString alias;
        QString description;
        QString maskcolor;
        int     lockstatus;
        //streamio
        int     oasis_nr;
        int     oasis_dt;
        int     gds_nr;
        int     gds_dt;
        //display
        bool    visible;
        bool    selectable;
        int     transparencyOrder;
        QPen    outline;
        QBrush  fill;
    };
    void clear();
    bool params(QXmlStreamReader *reader, QStringList *messageLog) override;
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    QList<Info> layers;
};
class OPS_CADLayersList : public Base {
public:
    OPS_CADLayersList()
        : Base(ops, "CADLayers") {}
    void clear();
    QList<OPS_CADLayer::Info>&& get();

    bool   params(QXmlStreamReader *reader, QStringList *messageLog) override;
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    OPS_CADLayer layer;
};
class OPS_DerivedLayer : public Base {
public:
    enum {INVALID, OR, AND, NOT, INTERACT };
    OPS_DerivedLayer() : Base(ops, "DerivedLayer") {}
    struct Info {
        QString name;
        QString description;

    };
    void clear();
    bool params(QXmlStreamReader *reader, QStringList *messageLog) override;
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    bool  endElement(QXmlStreamReader *reader, QStringList *messageLog) override;

    QList<Info> layers;
    bool        broken = false;
    int         depth = 0;
};
class OPS_DerivedLayersList : public Base {
    OPS_DerivedLayersList() : Base(ops, "DerivedLayers") {}
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) {
        Q_UNUSED(messageLog);
        if (layer.match(reader))
            return &layer;
        return nullptr;
    }
    OPS_DerivedLayer layer;
};
class OPS_Layers : public Base {
public:
    OPS_Layers() : Base(ops, "Layers") {}
    void clear() {

    }

};

} //namespace Import
} //namespace SI2
} //namespace CE
} //namespace K


/*******************************************/

/*******************************************/

/*******************************************/
/*******************************************/
/*******************************************/
/*******************************************/
/*******************************************/
/*******************************************/
/*******************************************/
/*******************************************/
/*******************************************
OPS_OPS
 |
 +--OPS_Technology
 |   |
 |   ...
 |
 +--OPS_LayerComponents (skipping, i dont undestand this
 |
 +--
 *******************************************/

/*******************************************/
class Matcher_OPS_OPS : public Base {
public:
    Matcher_OPS_OPS(QList<Tech*> * out, const QString& defname)
        : Base(ops, "OPS"), out_(out) {}
    bool params(QXmlStreamReader *reader, QStringList *messageLog) override {
        auto version = reader->attributes().value(QLatin1String("opsVersion"));
        if (version != QLatin1String("OPS_1.2"))
            log(reader, messageLog, "warning: unsupported OPS version");
        return true;
    }

    bool params(QXmlStreamReader *reader, QStringList *messageLog) override {
        auto version = reader->attributes().value(QLatin1String("opsVersion"));
        if (version != QLatin1String("OPS_1.2"))
            log(reader, messageLog, "warning: unsupported OPS version");
        return true;
    }
    MatcherBase * startElement(QXmlStreamReader *reader, QStringList *messageLog) override {
        if (tech.match(reader))
            return &tech;
        return nullptr;
    }
    MatcherBase * endElement(QXmlStreamReader *reader, QStringList *messageLog, MatcherBase **next) override {
        Q_UNUSED(reader);
        Q_UNUSED(messageLog);
        Q_UNUSED(next);
        *out_ = tech.mCompletedTechs;
        tech.mCompletedTechs.clear();
    }

    QList<Tech*> mCompletedTechs;
    QList<Tech*> mTechs;
    QString      name;
    Matcher_OPS_Technology tech;
    QList<Tech*> * out_;
};


}
