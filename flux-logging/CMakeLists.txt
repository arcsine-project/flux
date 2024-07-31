flux_interface_library(logging
    COMMON
        HEADERS
            "flux/logging/detail/logger_impl.hpp"
            "flux/logging/detail/source_location.hpp"
            "flux/logging/detail/strip_path.hpp"
            "flux/logging/logger.hpp"
            "flux/logging.hpp"
        LINK
            flux::io)

# code: language="CMake" insertSpaces=true tabSize=4