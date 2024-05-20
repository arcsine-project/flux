flux_static_library(foundation
    COMMON
        TEST
            "flux/foundation/memory/detail/constexpr_memcpy-test.cpp"
            "flux/foundation/memory/construct-test.cpp"
            "flux/foundation/memory/relocate-test.cpp"
            "flux/foundation/memory/uninitialized_algorithms-test.cpp"
            "flux/foundation/memory/uninitialized_storage-test.cpp"
        SOURCE
            "flux/foundation/dummy.cpp"
        LINK
            flux::io)

# code: language="CMake" insertSpaces=true tabSize=4