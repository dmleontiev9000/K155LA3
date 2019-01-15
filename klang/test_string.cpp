#include "test_string.h"

using namespace K::Lang;
StringTest::StringTest()
{
    qsrand(qHash(QDateTime::currentDateTime()));
}
StringTest::~StringTest()
{
}
void StringTest::allocatePool()
{
    Q_ASSERT(pool == nullptr);
    pool = new AssetPool();
}
void StringTest::destroyPool()
{
    delete pool;
    pool = nullptr;
}
void StringTest::Test(const char * s1, uint cat) {
    auto s = (const unsigned char *) s1;
    for(int i = 0; s[i]; ++i) {
        uint ss = S::sym(QChar::fromLatin1(s[i]));
        if (S::symtype(ss) != cat) {
            char cc = isprint(s[i]) ? s[i] : '?';
            qWarning("symbol %02x(%c) has wrong category", int(s[i]), cc);
        }
        QVERIFY(S::symtype(ss) == cat);
        QVERIFY(S::sym(ss) == s[i]);
    }
}
void StringTest::symtypeTest()
{
    Test("azAZ_$", S::SYM_LETTER);
    Test("0123456789", S::SYM_DIGIT);
    Test(".", S::SYM_DOT);
    Test(",", S::SYM_COMMA);
    Test("(", S::SYM_LBRACKET);
    Test(")", S::SYM_RBRACKET);
    Test("[", S::SYM_LINDEX);
    Test("]", S::SYM_RINDEX);
    Test("~@!%^&*-+/|<>", S::SYM_PUNCT);
    Test(" \t\r\n", S::SYM_SPACE);
    Test("=", S::SYM_EQUAL_SIGN);
    Test(":", S::SYM_SEMICOLON);
    Test("\"", S::SYM_STR2);
    Test("\'", S::SYM_STR1);
    Test("#", S::SYM_COMMENT);

}
void StringTest::simpleAllocation()
{
    Q_ASSERT(pool != nullptr);
    pool->lock();
    String * s1 = String::alloc(pool, 1);
    QVERIFY2(s1 != nullptr, "allocated string of 1 symbols");
    String * s2 = String::alloc(pool, 50);
    QVERIFY2(s2 != nullptr, "allocated string of 50 symbols");
    String * s3 = String::alloc(pool, 500);
    QVERIFY2(s3 != nullptr, "allocated string of 5000 symbols");
    String * s4 = String::alloc(pool, 3000);
    QVERIFY2(s4 != nullptr, "allocated string of 3000 symbols");
    pool->unlock();

    QThread::sleep(1);

    pool->lock();
    s1->dispose();
    s2->dispose();
    s3->dispose();
    s4->dispose();
    pool->unlock();

    QThread::sleep(2);
}
void StringTest::complexAllocation()
{
    unsigned char text[]={"0123456789abcdef"};
    Q_ASSERT(pool != nullptr);

    const int N = 1000;
    String * strs[N];
    for(int i = 0; i < N; ++i) strs[i] = nullptr;

    for (int loop = 0; loop < 5; ++loop) {
        pool->lock();
        for(int i = 0; i < N; ++i)
        {
            if (strs[i])
                continue;
            uint len = 1+(qrand() % 1000);
            strs[i] = String::alloc(pool, len);
            strs[i]->bind(&strs[i]);
            QVERIFY(strs[i] != nullptr);
            QVERIFY(strs[i]->string_length == len);
            for(uint j = 0; j < len; ++j)
                strs[i]->symbols[j] = text[j & 15];
        }
        pool->unlock();

        QThread::sleep(2);

        pool->lock();
        for(int i = 0; i < N; i+=3)
        {
            strs[i]->dispose();
            strs[i] = nullptr;
        }
        pool->unlock();

        QThread::sleep(2);

        pool->lock();
        for(int i = 0; i < N; ++i)
        {
            if (!strs[i])
                continue;
            bool ok = true;
            for(uint j = 0; j < strs[i]->string_length; ++j) {
                ok &= (strs[i]->symbols[j] == text[j & 15]);
                if (!ok) break;
            }
            if (!ok) {
                qDebug()<<"LEN="<<strs[i]->string_length;
                for(uint j = 0; j < qMin(20u, strs[i]->string_length); ++j) {
                    auto c = strs[i]->symbols[j];
                    if (c < '0' || c > 'z')
                        c = '?';
                    fprintf(stderr, "%c", c);
                }
                fprintf(stderr, "\r\n");
            }
            QVERIFY2(ok, "heap corruption test");
        }

        for(int i = 0; i < N; i+=2)
        {
            if (!strs[i])
                continue;
            strs[i]->dispose();
            strs[i] = nullptr;
        }
        pool->unlock();

        QThread::sleep(2);

        pool->lock();
        for(int i = 0; i < N; ++i)
        {
            if (!strs[i])
                continue;
            bool ok = true;
            for(uint j = 0; j < strs[i]->string_length; ++j)
                ok &= (strs[i]->symbols[j] == text[j & 15]);
            QVERIFY2(ok, "heap corruption test");
        }
        pool->unlock();

        QThread::sleep(2);
        qDebug()<<"iteration"<<loop<<"passed";
    }
}
QTEST_APPLESS_MAIN(StringTest)
