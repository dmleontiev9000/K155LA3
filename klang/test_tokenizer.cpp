#include <QtTest>
#include "../edit_utils/assetpool.h"

using namespace K::EditUtils;
class TokenizerTest : public QObject
{
    Q_OBJECT
public:
    TokenizerTest();
    ~TokenizerTest();
private Q_SLOTS:
    void allocateAndDestroy
private:
    AssetPool * pool;
};


TokenizerTest::Klang_testTest()
{
    pool = new AssetPool();
}
TokenizerTest::~TokenizerTest()
{
    delete pool;
}

void Klang_testTest::testGC()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(TokenizerTest)

#include "tst_klang_testtest.moc"
