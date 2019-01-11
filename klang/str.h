#pragma once

#include "klang_global.h"
#include <assetpool.h>

namespace K {
namespace Lang {
namespace S {

enum {
    COLOR_DEFAULT=0,
    COLOR_BAD,

    COLOR_KEYWORD,
    COLOR_IMMEDIATE,
    COLOR_STRING,

    COLOR_TYPE,
    COLOR_MEMBER,
    COLOR_ARGUMENT,
    COLOR_METHOD,
    COLOR_VIRTMETHOD,
    COLOR_COMMENT,
    COLOR_BRACKET,
    COLOR_INDEX,

    CERROR     =0x400,
    CWARNING   =0x200,
    CFOUND     =0x100,
    UNUSED     =0x080,
};
enum {
    SYM_BAD = 0,
    SYM_SPACE,
    SYM_LETTER,
    SYM_DIGIT,
    SYM_DOT,
    SYM_COMMA,
    SYM_SEMICOLON,
    SYM_EQUAL_SIGN,
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
struct K_LANG_EXPORT String : public K::EditUtils::Element {
    typedef quint32 Symbol;
    quint32 string_length;
    Symbol  symbols[0];
    
    void colorify(uint from, uint to, uint attr);
    void decolorify();

    static String * alloc(K::EditUtils::AssetPool * pool, uint len);
    void bind(String ** pointer) {
        if (pointer) *pointer = this;
        handle  = (Element**)pointer;
    }
};

namespace S {

inline quint32 sym(String::Symbol s) { return s & 0xffffu; }
inline quint32 symtype(String::Symbol s) { return (s>>16) & 0x1fu; }
inline quint32 symtype2(String::Symbol s) { return s & 0x1fffffu; }
inline quint32 symattr(String::Symbol s) { return (s>>21) & 0x7ffu; }
inline String::Symbol sym(String::Symbol s, quint32 attr)
{ return (String::Symbol)((s & 0x001fffff) | (attr<<21)); }
K_LANG_EXPORT String::Symbol sym(QChar c);

} //namespace S

} //namespace Lang
} //namespace K
