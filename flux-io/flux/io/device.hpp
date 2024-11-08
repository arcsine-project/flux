#pragma once
#include <fast_io_device.h>

namespace flux::io {

using ::fast_io::ibuf_file;
using ::fast_io::obuf_file;
using ::fast_io::open_mode;
using ::fast_io::output_stream;

using ::fast_io::ibuf_file_mutex;
using ::fast_io::obuf_file_mutex;

using ::fast_io::io_flush_guard;

using dir  = ::fast_io::dir_file;
using pipe = ::fast_io::pipe;

// clang-format off
template <typename CharT, typename T>
using reserve_type_t = ::fast_io::io_reserve_type_t<CharT, T>;
// clang-format on

} // namespace flux::io