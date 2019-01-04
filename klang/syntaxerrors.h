#pragma once

#include "documents_global.h"
#include <QtGlobal>

namespace K {
namespace Lang {

namespace SyntaxErrors {
enum : unsigned {
    no_error   = 0,
    //warnings
    //errors
    error_base = 0x80000000,
    incompatible_digit_for_integer_base,
    expected_integer_constant,
    floating_point_constant_out_of_range,
    integer_contant_is_too_large,
    invalid_characters_in_constant,
    invalid_operator,
    invalid_symbol,
    string_not_terminated,
    unexpected_token,
    unexpected_tokens_past_end,
};
inline bool is_error(unsigned e) { return e >= error_base; }
}

} //namespace Lang
} //namespace K
