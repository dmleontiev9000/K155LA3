#pragma once
#include "import_opdk.h"

namespace K {
namespace CE {
namespace OPDK {
namespace Import {

enum {INVALID, OTHER, NWELL, PWELL, NDIFF, PDIFF, NIMPLANT, PIMPLANT,
      POLY, CUT, METAL, CONTACTLESSMETAL, DIFFUSION, //RECOGNITION,
      PASSIVATIONCUT, NONE, CUSTOM };

int enumRole(const QStringRef& ref);
class OPS_RefLayer : public Base {
public:
    OPS_RefLayer(const char * name1)
        : Base(ops, name1) {}
    bool valid() { return !name.isEmpty(); }
    void clear() { name.clear(); }
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    QString   name;
};

class OPS_Value : public Base {
public:
    enum {Any, Positive, PositiveOrZero, String };
    OPS_Value(const char * name, int kind = Any)
        : Base(ops, name), kind_(kind) {}
    void clear() { valid = false; }
    bool params(QXmlStreamReader *reader, QStringList *messageLog) override;

    bool   valid;
    int    kind_;
    double value;
    QString stringvalue;
};

class SI_Unit : public Base {
public:
    enum Type { Invalid, None, Meter, Time, Frequency, Mass, Temperature, Resistivity, Capacity, Inductance, };

    SI_Unit(Type reqtype) : Base("opc", "SIUnit"), require(reqtype) {}
    void clear() { type = Invalid; }
    bool valid() const { return type != Invalid; }
    bool params(QXmlStreamReader *reader, QStringList *messageLog) override;

    Type    type;
    Type    require;
    int     line;
    int     exponent;
    double  multiplier;
    double  offset;
};

class Documentation : public Base {
public:
    Documentation() : Base(opc, "Documentation") {}
    Base * startElement(QXmlStreamReader *reader, QStringList *messageLog) override;
    QString description;
};

class DocReference : public Base {
public:
    DocReference() : Base (opc, "Reference") {}
    bool params(QXmlStreamReader *reader, QStringList *messageLog) override;
    QString href;
};

} //namespace Import
} //namespace SI2
} //namespace CE
} //namespace K
