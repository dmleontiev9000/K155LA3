#include "interfaces.h"
static K::IfCore * volatile self = nullptr;

K::IfCore::IfCore()
{
    Q_ASSERT(self == nullptr);
    self = this;
}
K::IfCore::~IfCore()
{
    self = nullptr;
}
K::IfCore * K::IfCore::instance()
{
    return self;
}
K::IfWorker::~IfWorker()
{
}

