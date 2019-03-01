#include "import_opdk_misc.h"

using namespace K::CE::OPDK::Import;

Base * OPS_RefLayer::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (reader->namespaceUri() != ops)
        return nullptr;
    auto n = reader->name();
    if (n == QLatin1String("RefCADLayerName") ||
        n == QLatin1String("RefCADLayerAlias") ||
        n == QLatin1String("RefDerivedLayer") ||
        n == QLatin1String("RefGenericLayer"))
    {
        name = reader->attributes().value("value").toString();
        if (name.isEmpty())
            log(reader, messageLog, "layer name is empty");
    }
    else
    {
        log(reader, messageLog, "unknown subelement type");
    }
    return nullptr;
}
bool OPS_Value::params(QXmlStreamReader *reader, QStringList *messageLog) {
    valid = false;
    auto v = reader->attributes().value("value");
    if (!v.isEmpty()) {
        if (kind_ == String) {
            stringvalue = v.toString();
            valid = true;
        } else {
            value = v.toDouble(&valid);
            if (value < 0.0 && kind_ != Any)
                valid = false;
            if (value <=0.0 && kind_ == Positive)
                valid = false;
        }
    }
    if (!valid)
        log(reader, messageLog, "wrong value");
    return valid;
}
bool SI_Unit::params(QXmlStreamReader *reader, QStringList *messageLog) {
    type     = Invalid;
    line     = reader->lineNumber();
    exponent = ~0;
    multiplier= 1.0;
    offset   = 0;

    Type t;
    auto siname = reader->attributes().value("siName");
    if (siname.length() == 1) {
        switch(siname[0].unicode()) {
        case u'm': t = Meter; break;
        case u's': t = Time; break;
        case u'E': t = Meter; exponent = -10; break;
        case u'F': t = Capacity; break;
        case u'K': t = Temperature; break;
        case u'Ω': t = Resistivity; break;
        default: break;
        }
    } else if (siname.length() == 2) {
        if (siname == "℃") {
            t = Temperature;
            offset = 273.0;
        }
    }
    if (t == Invalid) {
        log(reader, messageLog, "unsupported unit type");
        return false;
    }
    if (require != Invalid && require != t) {
        log(reader, messageLog, "wrong unit type");
        return false;
    }

    auto pf = reader->attributes().value("prefix");
    if (pf.size() == 0) {
        exponent = 0;
    } else if (pf.size() == 1) {
        switch(pf[0].unicode()) {
        case 'Y': exponent = 24; break;
        case 'Z': exponent = 21; break;
        case 'E': exponent = 18; break;
        case 'P': exponent = 15; break;
        case 'T': exponent = 12; break;
        case 'G': exponent = 9;  break;
        case 'M': exponent = 6;  break;
        case 'k': exponent = 3;  break;
        case 'h': exponent = 2;  break;
        case 'd': exponent =-1;  break;
        case 'c': exponent =-2;  break;
        case 'm': exponent =-3;  break;
        case u'µ':exponent =-6;  break;
        case 'n': exponent =-9;  break;
        case 'p': exponent =-12; break;
        case 'f': exponent =-15; break;
        case 'a': exponent =-18; break;
        case 'z': exponent =-21; break;
        case 'y': exponent =-24; break;
        default:;
        }
    } else if (pf.size() == 2) {
        if (pf == "da")
            exponent = 1;
    }
    if (exponent == ~0) {
        log(reader, messageLog, "wrong prefix");
        return false;
    }

    bool ok = false;
    auto ex = reader->attributes().value("exponent");
    if (!ex.isEmpty()) {
        multiplier = reader->attributes().value("exponent").toDouble(&ok);
        if (!ok || multiplier <= 0.0) {
            log(reader, messageLog, "wrong exponent");
            return false;
        }
    }
    type = t;
    return true;
}

Base * Documentation::startElement(QXmlStreamReader *reader, QStringList *messageLog) {
    if (reader->namespaceUri() != opc)
        return nullptr;
    if (reader->name() == QLatin1String("Description")) {
        description = reader->readElementText(QXmlStreamReader::IncludeChildElements);
    }
    log(reader, messageLog, "unsupported element");
    return nullptr;
}

bool DocReference::params(QXmlStreamReader *reader, QStringList *messageLog) {
    Q_UNUSED(messageLog);
    href = reader->attributes().value("href").toString();
    return true;
}
