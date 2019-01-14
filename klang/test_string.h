#pragma once
#include <QtTest>
#include <QDateTime>
#include "../editutils/assetpool.h"
#include "str.h"
using namespace K::EditUtils;
using namespace K::Lang;
class StringTest : public QObject
{
    Q_OBJECT
public:
    StringTest();
    virtual ~StringTest();
private Q_SLOTS:
    void allocatePool();
    void symtypeTest();
    void simpleAllocation();
    void complexAllocation();
    void destroyPool();
private:
    AssetPool * pool = nullptr;
    void Test(const char *s, uint cat);
};
