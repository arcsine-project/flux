// IWYU pragma: private, include "../meta.hpp"
#pragma once

namespace flux::meta {

constexpr auto prvalue(auto&& arg) {
    return arg;
}

} // namespace flux::meta
