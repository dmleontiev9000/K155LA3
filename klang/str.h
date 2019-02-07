#pragma once

#include "klang_global.h"
#include "assetpool.h"

namespace K {
namespace Lang {
namespace S {

enum {
    SYM_BAD = 0,
    SYM_SPACE,
    SYM_LETTER,
    SYM_DIGIT,
    SYM_DOT,
    SYM_COMMA,
    SYM_SEMICOLON,
    SYM_EQUAL_SIGN,
    SYM_COMMENT,
    SYM_PUNCT,
    SYM_LBRACKET,
    SYM_RBRACKET,
    SYM_LINDEX,
    SYM_RINDEX,
    SYM_STR1,
    SYM_STR2,
    MAX_SYM
};
Q_STATIC_ASSERT(MAX_SYM < 32);

} //namespace S

/*
 * do not attempt to store pointers to anywhere except the node it belongs to.
 * there must be exactly one live pointer to string while context is not bound
 * and it must reside in the node. once context is locked, you can make temporary
 * copies of this pointer
 */
class K_LANG_EXPORT String : public K::EditUtils::Element {
public:
    typedef quint32 Symbol;
    static String * alloc(K::EditUtils::AssetPool * pool, uint len);
    static String * alloc(K::EditUtils::AssetPool * pool, const QString& text);
    static String * alloc(K::EditUtils::AssetPool * pool, const QLatin1String& text);
    static String * alloc(K::EditUtils::AssetPool * pool, const String * other,
                          quint32 from, quint32 to);
    Symbol at(uint n) const { return symbols[n]; }
    uint length() const { return string_length; }
    uint hash(uint from, uint to) const;

    bool equal(const String* str, uint from, uint to);

    void colorify(uint from, uint to, uint attr);
    void decolorify();

    QString substr(uint from, uint to);
private:
    quint32 string_length;
    quint32 misc;
    Symbol  symbols[0];
};

namespace S {
enum {HKEY_LEN = 7, HKEY_MASK=(1<<HKEY_LEN)-1};
inline quint32 sym(String::Symbol s) { return s & 0xffffu; }
inline quint32 symtype(String::Symbol s) { return (s>>16) & 0x1fu; }
inline quint32 symtype2(String::Symbol s) { return s & 0x1fffffu; }
inline quint32 symattr(String::Symbol s) { return (s>>21) & 0x7ffu; }
inline String::Symbol sym(String::Symbol s, quint32 attr)
{ return (String::Symbol)((s & 0x001fffff) | (attr<<21)); }
String::Symbol sym(QChar c);

} //namespace S

} //namespace Lang
} //namespace K
