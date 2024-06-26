#include <flux/foundation/memory/debugging.hpp>
#include <flux/io.hpp>

#include <atomic>

namespace flux::fou {

constexpr void print_define(io::reserve_type_t<char, allocator_info>, io::output_stream auto stream,
                            allocator_info const& info) noexcept {
    io::print(stream, "Allocator ", info.name, " (at ", io::address(info.allocator), ") ");
}

namespace {

void default_leak_handler(allocator_info const& info, ::std::ptrdiff_t amount) noexcept {
    if (amount > 0) {
        io::panic(info, "leaked ", amount, " bytes");
    } else {
        io::panic(info, "has deallocated ", amount, " bytes more than ever allocated");
    }
}

::std::atomic<leak_handler> internal_leak_handler(default_leak_handler);

} // namespace

leak_handler set_leak_handler(leak_handler handler) noexcept {
    return internal_leak_handler.exchange(handler ? handler : default_leak_handler);
}

leak_handler get_leak_handler() noexcept {
    return internal_leak_handler;
}

namespace {

void default_invalid_ptr_handler(allocator_info const& info, const void* ptr) noexcept {
    io::panic("Deallocation function of ", info, "received invalid pointer ", io::address(ptr));
}

::std::atomic<invalid_pointer_handler> internal_invalid_ptr_handler(default_invalid_ptr_handler);

} // namespace

invalid_pointer_handler set_invalid_pointer_handler(invalid_pointer_handler handler) noexcept {
    return internal_invalid_ptr_handler.exchange(handler ? handler : default_invalid_ptr_handler);
}

invalid_pointer_handler get_invalid_pointer_handler() noexcept {
    return internal_invalid_ptr_handler;
}

namespace {

void default_buffer_overflow_handler(const void* memory, ::std::size_t node_size,
                                     const void* ptr) noexcept {
    io::panic("Buffer overflow at address ", io::address(ptr),
              " detected, corresponding memory block", io::address(memory), " has only size ",
              node_size);
}

::std::atomic<buffer_overflow_handler>
        internal_buffer_overflow_handler(default_buffer_overflow_handler);

} // namespace

buffer_overflow_handler set_buffer_overflow_handler(buffer_overflow_handler handler) noexcept {
    return internal_buffer_overflow_handler.exchange(handler ? handler
                                                             : default_buffer_overflow_handler);
}

buffer_overflow_handler get_buffer_overflow_handler() noexcept {
    return internal_buffer_overflow_handler;
}

} // namespace flux::fou