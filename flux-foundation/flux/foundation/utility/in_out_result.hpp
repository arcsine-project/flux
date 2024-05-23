#pragma once

namespace flux::fou {

// clang-format off
template <typename In, typename Out>
struct [[nodiscard]] in_out_result final {
    FLUX_NO_UNIQUE_ADDRESS In  in;
    FLUX_NO_UNIQUE_ADDRESS Out out;

    template <typename I, typename O>
        requires meta::convertible_to<In  const&, I> and
                 meta::convertible_to<Out const&, O>
    constexpr operator in_out_result<I, O>() const& noexcept {
        return {in, out};
    }

    template <typename I, typename O>
        requires meta::convertible_to<In , I> and
                 meta::convertible_to<Out, O>
    constexpr operator in_out_result<I, O>() && noexcept {
        return {::std::move(in), ::std::move(out)};
    }
};
// clang-format on

} // namespace flux::fou