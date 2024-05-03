#pragma once
#include <fast_io_device.h>

namespace flux::io {

using ::fast_io::ibuf_file;
using ::fast_io::obuf_file;
using ::fast_io::open_mode;

using ::fast_io::ibuf_file_mutex;
using ::fast_io::obuf_file_mutex;

using ::fast_io::io_flush_guard;

using dir  = ::fast_io::dir_file;
using pipe = ::fast_io::pipe;

} // namespace flux::io