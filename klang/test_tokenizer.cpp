#include "test_tokenizer.h"
#include "str.h"
#include "tokenizer.h"
#include <QDebug>
#include <QDateTime>
using namespace K::Lang;
using namespace K::Lang::T;
TokenizerTest::TokenizerTest()
{
    qsrand(qHash(QDateTime::currentDateTime()));    
}
TokenizerTest::~TokenizerTest()
{
}
void TokenizerTest::allocatePool()
{
    Q_ASSERT(pool == nullptr);
    pool = new AssetPool();
}
void TokenizerTest::destroyPool()
{
    delete pool;
    pool = nullptr;
}
enum {
    KEYWORD_MIN = T::TOKEN_MAX,
    //keywords
    KEYWORD_VOID,
    KEYWORD_INT,
};
static Tokenizer::KW keywords[] = {
    _KW("void",     KEYWORD_VOID),
    _KW("int?",     KEYWORD_INT),
    {0,0,0,0}
};
const char * tokname(uint t) {
    switch(t) {
    case T::TOKEN_INVALID: return "invalid";
    case T::TOKEN_ERROR: return "error";
    case T::TOKEN_LBR: return "lbr";
    case T::TOKEN_RBR: return "rbr";
    case T::TOKEN_LIDX: return "lidx";
    case T::TOKEN_RIDX: return "ridx";
    case T::TOKEN_COMMA: return "comma";
    case T::TOKEN_DOT: return "dot";
    case T::TOKEN_DOT3: return "dot3";
    case T::TOKEN_IDENT: return "ident";
    case T::TOKEN_INT: return "int";
    case T::TOKEN_FLOAT: return "float";
    case T::TOKEN_SC: return "semicolon";
    case T::TOKEN_SC2: return "::";
    case T::TOKEN_STR1: return "string('')";
    case T::TOKEN_STR2: return "string(\"\")";
    case T::TOKEN_ASSIGN: return "assingment";
    case T::TOKEN_OPERATOR: return "operator";
    default: return "fail";
    }
}
void TokenizerTest::test(const char *text,
                         uint n,
                         const char **tokens,
                         const uint *ids,
                         const QVariant **vars)
{
    pool->lock();
    auto s = String::alloc(pool, QLatin1String(text));
    QVERIFY(s != nullptr);

    Tokenizer::Context ctx;
    Tokenizer tok(keywords);

    uint count = 0;
    while(tok.tokenize(s, &ctx)) {
        QVERIFY2(count < n, "no unexpected tokens");
        QVERIFY(ctx.start() < s->string_length);
        QVERIFY(ctx.end() <= s->string_length);
        QVERIFY(ctx.start() < ctx.end());
        //qDebug("%s", text+ctx.start());
        auto len = ctx.end() - ctx.start();
        char tmp[len+1];
        for(uint n = 0; n < len; ++n)
            tmp[n] = S::sym(s->symbols[ctx.start()+n]);
        tmp[len] = 0;

        if (!strcmp(tmp, tokens[count]) == 0) {
            qDebug("TOKEN:   '%s' != '%s'", tmp, tokens[count]);
            QVERIFY2(false, "wrong token");
        }

        if (ids[count] != ctx.token()) {
            qDebug("TOKEN ID: %s != %s", tokname(ctx.token()), tokname(ids[count]));
            QVERIFY2(false, "wrong token");
        }

        if (vars &&
            vars[count] &&
            (*(vars[count])) != ctx.data())
        {
            qDebug("VALUE:   %s != %s",
                   qPrintable(ctx.data().toString()),
                   qPrintable(vars[count]->toString()));
            QVERIFY2(false, "wrong token");
        }
        ctx.next();
        ++count;
    }
    QVERIFY2(count == n, "no missing tokens");
    QVERIFY2(ctx.end() == s->string_length, "tokenizer reached end");
    s->dispose();
    pool->unlock();
}

void TokenizerTest::tokenizeTest() {
    const char * toks1[]={
        "ab012","100","[","]",
        "(","(",")",")",
        "_1","--","-5.5",
        "+=",">>=","+",
        "*=","==",",",
        "=", ".", "...",

    };
    const uint tids1[] = {
        TOKEN_IDENT, TOKEN_INT, TOKEN_LIDX, TOKEN_RIDX,
        TOKEN_LBR, TOKEN_LBR, TOKEN_RBR, TOKEN_RBR,
        TOKEN_IDENT, TOKEN_OPERATOR, TOKEN_FLOAT,
        TOKEN_OPERATOR, TOKEN_OPERATOR, TOKEN_OPERATOR,
        TOKEN_OPERATOR, TOKEN_OPERATOR, TOKEN_COMMA,
        TOKEN_ASSIGN, TOKEN_DOT, TOKEN_DOT3,
    };
    Q_STATIC_ASSERT((sizeof(toks1)/sizeof(toks1[0])) ==
             (sizeof(tids1)/sizeof(tids1[0])));
    test("ab012 100  \t  [] (()) _1 -- -5.5 += >>=+*===,="
         ". ...",
         sizeof(toks1)/sizeof(toks1[0]), toks1, tids1, nullptr);
}
void TokenizerTest::integerTest() {
    QVariant one(1);
    QVariant c33(65);
    const char * toks1[] = {
        "1","0b1","0o1","0x1",
        "65", "0b1000001", "0o101","0x41",
        "1aa", "0b12", "0o39", "0xah",
        "999999999999999999999",
    };
    const uint tids1[] = {
        TOKEN_INT,TOKEN_INT,TOKEN_INT,TOKEN_INT,
        TOKEN_INT,TOKEN_INT,TOKEN_INT,TOKEN_INT,
        TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR,
        TOKEN_ERROR,

    };
    const QVariant * vars[] ={
        &one,&one,&one,&one,
        &c33,&c33,&c33,&c33,
        nullptr, nullptr, nullptr, nullptr,
        nullptr,
    };
    Q_STATIC_ASSERT((sizeof(toks1)/sizeof(toks1[0])) ==
             (sizeof(tids1)/sizeof(tids1[0])));
    test("1 " "0b1 " "0o1  " "0x1 "
         "65 " "0b1000001 " "0o101 ""0x41 "
         "1aa " "0b12 " "0o39 " "0xah "
         "999999999999999999999",
         sizeof(toks1)/sizeof(toks1[0]), toks1, tids1, vars);
}
void TokenizerTest::integerTest2() {
    char tmp[64];
    for(int i = 0; i < 32; ++i) {
        unsigned long long l = 0;
        l = rand(); l<<=48; l += qrand();
        sprintf(tmp, "0x%llxlu", l);
        QVariant v(l);

        const char * toks1[] = {tmp};
        uint tids[] = {TOKEN_INT};
        const QVariant * vars[] = {&v};

        test(tmp, 1, toks1, tids, vars);
    }    
}
void TokenizerTest::floatTest() {
    char tmp[128];
    for(int i = 0; i < 32; ++i) {
        sprintf(tmp, "%d.%d", rand(), rand());
        QVariant v(tmp);
        QVariant d(v.toDouble());

        const char * toks1[] = {tmp};
        uint tids[] = {TOKEN_FLOAT};
        const QVariant * vars[] = {&d};

        test(tmp, 1, toks1, tids, vars);
    }
    for(int i = 0; i < 32; ++i) {
        int e = rand() % 200;
        sprintf(tmp, "%d.%de%d", rand(), rand(), e-100);
        QVariant v(tmp);
        QVariant d(v.toDouble());

        const char * toks1[] = {tmp};
        uint tids[] = {TOKEN_FLOAT};
        const QVariant * vars[] = {&d};

        test(tmp, 1, toks1, tids, vars);
    }
}
void TokenizerTest::expressionTest() {
    const char * toks1[]={
        "aa$_","+","(","-0x440",
        "*","-5.432f","+",
        "12",")",">>=",
        "ff"
    };
    const uint tids1[] = {
        TOKEN_IDENT, TOKEN_OPERATOR, TOKEN_LBR, TOKEN_INT,
        TOKEN_OPERATOR, TOKEN_FLOAT, TOKEN_OPERATOR,
        TOKEN_INT, TOKEN_RBR, TOKEN_OPERATOR,
        TOKEN_IDENT
    };
    Q_STATIC_ASSERT((sizeof(toks1)/sizeof(toks1[0])) ==
             (sizeof(tids1)/sizeof(tids1[0])));
    QVariant i1(0x440), i2(-5.432f), i3(12);
    const QVariant * vars[] = {
        nullptr, nullptr, nullptr, &i1,
        nullptr, &i2, nullptr,
        &i3, nullptr, nullptr,
        nullptr
    };

    test("aa$_""+""(""-0x440"
         "*""-5.432f""+"
         "12"")"">>="
         "ff",
         sizeof(toks1)/sizeof(toks1[0]), toks1, tids1, vars);
}

QTEST_APPLESS_MAIN(TokenizerTest)

