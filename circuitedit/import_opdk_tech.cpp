#include "import_opdk_tech.h"

using namespace K::CE::OPDK::Import;

void OPS_TechnologySettingsDefault::clear() {
    grid.clear();
    siunit.clear();
}
Base * OPS_TechnologySettingsDefault::startElement(QXmlStreamReader *reader, QStringList *messageLog)  {
    if (grid.match(reader))
        return &grid;
    if (siunit.match(reader))
        return &siunit;
    log(reader, messageLog, "unsupported element");
    return nullptr;
}
bool OPS_TechnologySettingsDefault::endElement(QXmlStreamReader *reader, QStringList *messageLog)  {
    Q_UNUSED(reader);
    Q_UNUSED(messageLog);
    valid = grid.valid && siunit.valid();
    return valid;
}
void OPS_Option::clear() {
    names.clear();
    docs.clear();
    refs.clear();
}
bool OPS_Option::params(QXmlStreamReader *reader, QStringList *messageLog) {
    Q_UNUSED(messageLog);
    documentation.description.clear();
    reference.href.clear();

    auto name = reader->attributes().value("name");
    bool ok = !name.isEmpty();
    if (ok) names.append(name.toString());
    return ok;
}
Base * OPS_Option::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (documentation.match(reader))
        return &documentation;
    if (reference.match(reader))
        return &reference;
    log(reader, messageLog, "unsupported element");
    return nullptr;
}
bool OPS_Option::endElement(QXmlStreamReader *reader, QStringList *messageLog) {
    Q_UNUSED(reader);
    Q_UNUSED(messageLog);
    docs.append(documentation.description);
    refs.append(reference.href);
    return true;
}
void OPS_GenericLayerElement::clear() {
    valid = false;
    actual.clear();
    reference.clear();
    actual_list.clear();
    reference_list.clear();
}
Base * OPS_GenericLayerElement::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (actual.match(reader))
        return &actual;
    if (reference.match(reader))
        return &reference;
    log(reader, messageLog, "unsupported element");
    return nullptr;
}
bool OPS_GenericLayerElement::endElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (!actual.valid()) {
        log(reader, messageLog, "actual layer was not set");
        return false;
    }
    actual_list.append(std::move(actual.name));
    reference_list.append(std::move(reference.name));
    valid = true;
    return true;
}
void OPS_GenericLayer::clear() {
    layers.clear();
    element.clear();
}
bool OPS_GenericLayer::params(QXmlStreamReader *reader, QStringList *messageLog) {
    element.clear();
    auto name = reader->attributes().value("name").toString();
    if (name.isEmpty()) {
        log(reader,messageLog,"Layer name is empty");
        return false;
    }
    auto type = enumRole(reader->attributes().value("enumType"));
    if (type == INVALID) {
        log(reader, messageLog, "invalid layer type");
        return false;
    }
    Info info;
    info.type = type;
    info.name = name;
    layers.append(info);
    return true;
}
Base * OPS_GenericLayer::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (element.match(reader))
        return &element;
    log(reader, messageLog, "unsupported element");
    return nullptr;
}
bool OPS_GenericLayer::endElement(QXmlStreamReader *reader, QStringList *messageLog) {
    Q_UNUSED(reader);
    Q_UNUSED(messageLog);
    if (!element.valid) {
        layers.pop_back();
        return false;
    }
    layers.last().actual = std::move(element.actual_list);
    layers.last().refs = std::move(element.reference_list);
    return true;
}
void OPS_GenericLayers::clear() {
    layer.clear();
}
const QList<OPS_GenericLayer::Info>& OPS_GenericLayers::info() const {
    return layer.layers;
}
Base * OPS_GenericLayers::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (layer.match(reader))
        return &layer;
    log(reader, messageLog, "unsupported element");
    return nullptr;
}
void OPS_ConnectObject::clear() {
    entries.clear();
    fromLayer.clear();
    toLayer.clear();
    viaLayer.clear();
}
bool OPS_ConnectObject::params(QXmlStreamReader *reader, QStringList *messageLog) {
    fromLayer.clear();
    toLayer.clear();
    viaLayer.clear();

    Entry entry;
    bool ok;
    entry.ordering = reader->attributes().value("connectOrdering").toInt(&ok);
    if (!ok) {
        log(reader, messageLog, "invalid connection order");
        return false;
    }
    auto connectby = reader->attributes().value("connectBy");
    if (connectby == QLatin1String("via")) {
        entry.type = Type::Via;
    } else if (connectby == QLatin1String("touch")) {
        entry.type = Type::Touch;
    } else if (connectby == QLatin1String("substrate")) {
        entry.type = Type::Substrate;
    } else {
        log(reader, messageLog, "invalid connection type");
        return false;
    }
    entries.append(entry);
    return true;
}
Base * OPS_ConnectObject::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (fromLayer.match(reader))
        return &fromLayer;
    if (toLayer.match(reader))
        return &toLayer;
    if (viaLayer.match(reader))
        return &viaLayer;
    log(reader, messageLog, "unsupported element");
    return nullptr;
}
bool OPS_ConnectObject::endElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (!fromLayer.valid() || !toLayer.valid()) {
        log(reader,messageLog, "connection description misses layer references");
        entries.pop_back();
        return false;
    }
    entries.last().fromLayer = fromLayer.name;
    entries.last().toLayer = toLayer.name;
    if (viaLayer.valid())
        entries.last().viaLayer = viaLayer.name;
    return true;
}
void OPS_ConnectivityTable::clear() {
    connobj.clear();
}
QList<OPS_ConnectObject::Entry>&& OPS_ConnectivityTable::entries() {
    return std::move(connobj.entries);
}
Base * OPS_ConnectivityTable::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    Q_UNUSED(messageLog);
    if (connobj.match(reader))
        return &connobj;
    log(reader, messageLog, "unsupported element");
    return nullptr;
}
void OPS_MetalStack::clear() {
    stacks.clear();
}
bool OPS_MetalStack::valid() {
    return !stacks.isEmpty();
}
bool OPS_MetalStack::params(QXmlStreamReader *reader, QStringList *messageLog) {
    Q_UNUSED(messageLog);
    MetalStack ms;
    conntable.clear();
    documentation.description.clear();
    reference.href.clear();

    ms.name = reader->attributes().value("name").toString();
    if (ms.name.isEmpty()) {
        ms.name  = QChar('#');
        ms.name += QString::number(stacks.size());
    }
    int dupcount = 0;
    for(auto i = stacks.constBegin(); i != stacks.constEnd(); ++i)
        if (ms.name == i->name)
            ++dupcount;
    if (dupcount) {
        ms.name += QChar('#');
        ms.name += QString::number(dupcount);
    }
    stacks.append(ms);
    return true;
}
Base * OPS_MetalStack::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (conntable.match(reader))
        return &conntable;
    if (documentation.match(reader))
        return &documentation;
    if (reference.match(reader))
        return &reference;
    log(reader, messageLog, "unsupported element");
    return nullptr;
}
bool OPS_MetalStack::endElement(QXmlStreamReader *reader, QStringList *messageLog) {
    stacks.last().connections = conntable.entries();
    if (stacks.last().connections.isEmpty()) {
        stacks.pop_back();
        log(reader, messageLog, "Metal stack description invalid");
        return false;
    }
    return true;
}
void OPS_Technology::clear() {
    valid = false;
    settings_def.clear();
    metal_stack.clear();
    options.clear();
    documentation.description.clear();
    reference.href.clear();
}
Base * OPS_Technology::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    Q_UNUSED(messageLog);
    if (settings_def.match(reader))
        return &settings_def;
    if (metal_stack.match(reader))
        return &metal_stack;
    if (documentation.match(reader))
        return &documentation;
    if (reference.match(reader))
        return &reference;
    return nullptr;
}
bool OPS_Technology::endElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (!settings_def.valid ||
        !metal_stack.valid())
    {
        log(reader, messageLog, "Technology description is incomplete");
    }
    else
    {
        valid = true;
    }
    return valid;
}
