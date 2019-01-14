#pragma once
#include <QtTest>
#include "../editutils/assetpool.h"

using namespace K::EditUtils;
class TokenizerTest : public QObject
{
    Q_OBJECT
public:
    TokenizerTest();
    ~TokenizerTest();
private Q_SLOTS:
    void allocatePool();
    void tokenizeTest();
    void integerTest();
    void integerTest2();
    void floatTest();
    void expressionTest();
    void destroyPool();
private:
    AssetPool * pool = nullptr;
    void test(const char * text,
              uint         n,
              const char **tokens,
              const uint * ids,
              const QVariant ** vars);
};
