#include "import_opdk_layers.h"

using namespace K::CE::OPDK::Import;

static int mixType(const QStringRef& type) {
    if (type == QLatin1String("NOT"))
        return OPS_DerivedLayer::NOT;
    if (type == QLatin1String("AND"))
        return OPS_DerivedLayer::AND;
    if (type == QLatin1String("OR"))
        return OPS_DerivedLayer::OR;
    if (type == QLatin1String("INTERACT"))
        return OPS_DerivedLayer::INTERACT;
    return OPS_DerivedLayer::INVALID;
}
void OPS_CADLayer::clear() { layers.clear(); }
bool OPS_CADLayer::params(QXmlStreamReader *reader, QStringList *messageLog)  {
    Info info;
    auto a = reader->attributes();

    info.role       = enumRole(a.value("enumRole"));
    info.exponumber = a.value("expoNumber").toInt();
    info.name       = a.value("name").toString();
    info.alias      = a.value("alias").toString();
    info.description= a.value("description").toString();
    info.maskcolor  = a.value("maskColor").toString();
    info.lockstatus = NONE;
    info.oasis_nr   = ~0;
    info.gds_nr     = ~0;
    info.oasis_dt   = 0;
    info.gds_dt     = 0;
    info.visible    = false;
    info.selectable = false;
    info.transparencyOrder = 0;
    info.outline;
    info.fill;

    auto v = a.value("lockStatus");
    if (info.name.isEmpty() &&
        info.alias.isEmpty())
    {
        log(reader, messageLog, "layer has no name and no alias");
        return false;
    }
    if (v==QLatin1String("unlocked"))
        info.lockstatus = UNLOCKED;
    else if (v==QLatin1String("locked"))
        info.lockstatus = LOCKED;
    layers.append(info);
    return true;
}
Base * OPS_CADLayer::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (reader->namespaceUri() != ops)
        return nullptr;
    Info& info = layers.last();
    if (reader->name() == QLatin1String("StreamIO")) {
        auto fmt = reader->attributes().value("format");
        auto num = reader->attributes().value("number").toInt();
        auto dt  = reader->attributes().value("dataType").toInt();
        if (fmt == QLatin1String("GDSII")) {
            info.gds_nr = num;
            info.gds_dt = dt;
        } else if (fmt = QLatin1String("OASIS")) {
            info.oasis_nr = num;
            info.oasis_dt = dt;
        }
    } else if (reader->name() == QLatin1String("Display")) {
        info.visible = reader->attributes().value("visible") == "true";
        info.selectable = reader->attributes().value("selectable") == "true";
        info.transparencyOrder = reader->attributes().value("transparencyOrder").toInt();
        auto outline = reader->attributes().value("outline");
        if (!outline.isEmpty()) {
            QColor color;
            color.setNamedColor(outline.toString());
            if (color.isValid())
                info.outline.setColor(color);
        }
        auto fill = reader->attributes().value("fill");
        if (!fill.isEmpty()) {
            QColor color;
            color.setNamedColor(fill.toString());
            if (color.isValid())
                info.fill.setColor(color);
        }
    } else {
        log(reader, messageLog, "unsupported element");
    }
    return nullptr;
}
void OPS_CADLayersList::clear() {
    layer.clear();
}
QList<OPS_CADLayer::Info>&& OPS_CADLayersList::get() {
    return std::move(layer.layers);
}

bool   OPS_CADLayersList::params(QXmlStreamReader *reader, QStringList *messageLog) {
    Q_UNUSED(reader);
    Q_UNUSED(messageLog);
    layer.clear();
    return true;
}
Base * OPS_CADLayersList::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (layer.match(reader))
        return &layer;
    log(reader, messageLog, "unsupported element");
    return nullptr;
}

void OPS_DerivedLayer::clear() {
    layers.clear();
    depth = 0;
}
bool OPS_DerivedLayer::params(QXmlStreamReader *reader, QStringList *messageLog) {
    if (!depth) {
        Info info;
        info.name = reader->attributes().value("name").toString();
        info.description = reader->attributes().value("description").toString();
        if (info.name.isEmpty()) {
            log(reader,messageLog, "layer has no name");
            return false;
        }
        layers.append(info);
        broken = false;
        ++depth;
    }
    return true;
}
Base * OPS_DerivedLayer::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (broken) return nullptr;
    if (reader->namespaceUri() != ops) {
        log(reader, messageLog, "unsupported tag");
        if (reader->namespaceUri() == opc &&
            reader->name() == QLatin1String("Parameter")) {

        }
        return nullptr;
    }
    if (reader->name() == QLatin1String("Template")) {
        auto type = mixType(reader->attributes().value("name"));
        //NOT,AND,OR,INTERACT
        if (type == INVALID) {
            log(reader, messageLog, "Invalid or unsupported template type");
            broken = true;
            return nullptr;
        }
        auto
    } else if (reader->name() == QLatin1String("DerivedLayer")) {


    } else if (reader->name() == QLatin1String("RefCADLayerAlias")) {
        //
    } else if (reader->name() == QLatin1String("RefCADLayerName")) {
        //
    } else if (reader->name() == QLatin1String("RefDerivedLayer")) {
        //
    } else if (reader->name() == QLatin1String("RefGenericLayer")) {
        //

    } else {
        log(reader, messageLog, "unsupported element");
        return nullptr;
    }
    ++depth;
    return this;
}
bool  endElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (depth == 0) {
    } else {
        --depth;
    }
}
