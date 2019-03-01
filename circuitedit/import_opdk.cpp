#include "import_opdk.h"
#include "import_opdk_misc.h"
#include "import_opdk_layers.h"
#include "import_opdk_tech.h"
#include "import_opdk_rules.h"
#include "import_opdk_process.h"

QLatin1String K::CE::OPDK::Import::Base::opc("opc");
QLatin1String K::CE::OPDK::Import::Base::ops("ops");

int K::CE::OPDK::Import::enumRole(const QStringRef& et) {
    int type = INVALID;
    if (et == "other") type = OTHER; else
    if (et == "nWell") type = NWELL; else
    if (et == "pWell") type = PWELL; else
    if (et == "nDiff") type = NDIFF; else
    if (et == "pDiff") type = PDIFF; else
    if (et == "nImplant") type = NIMPLANT; else
    if (et == "pImplant") type = PIMPLANT; else
    if (et == "poly") type = POLY; else
    if (et == "cut") type = CUT; else
    if (et == "metal") type = METAL; else
    if (et == "contactlessMetal") type = CONTACTLESSMETAL; else
    if (et == "diffusion") type = DIFFUSION; else
    if (et == "recognition") type = RECOGNITION; else
    if (et == "passivationCut") type = PASSIVATIONCUT; else
    if (et == "node") type = NONE; else
    if (et == "CUSTOM") type = CUSTOM;
    return type;
}
