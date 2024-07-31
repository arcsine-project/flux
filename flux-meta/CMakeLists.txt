# flux_interface_library(meta
#     COMMON
#         LINK
#             flux::config)

xxx_flux_interface_library(meta
    COMMON
        HEADERS
            "flux/meta/compare.hpp"
            "flux/meta/concepts.hpp"
            "flux/meta/datasizeof.hpp"
            "flux/meta/declval.hpp"
            "flux/meta/integer.hpp"
            "flux/meta/prvalue.hpp"
            "flux/meta/type_traits.hpp"
            "flux/meta.hpp"
        LINK
            flux::config)

# code: language="CMake" insertSpaces=true tabSize=4