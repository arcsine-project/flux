flux_static_library(foundation
    COMMON
        TEST
            "flux/foundation/memory/detail/constexpr_memcpy-test.cpp"
            "flux/foundation/memory/detail/debug_helpers-test.cpp"
            "flux/foundation/memory/detail/fixed_stack-test.cpp"
            "flux/foundation/memory/detail/free_list_array-test.cpp"
            "flux/foundation/memory/detail/free_list-test.cpp"
            "flux/foundation/memory/align-test.cpp"
            "flux/foundation/memory/construct-test.cpp"
            "flux/foundation/memory/memory_block-test.cpp"
            "flux/foundation/memory/relocate-test.cpp"
            "flux/foundation/memory/static_allocator-test.cpp"
            "flux/foundation/memory/uninitialized_algorithms-test.cpp"
            "flux/foundation/memory/uninitialized_storage-test.cpp"
        SOURCE
            "flux/foundation/memory/detail/debug_helpers.cpp"
            "flux/foundation/memory/debugging.cpp"
        LINK
            flux::io
            flux::platform)

# code: language="CMake" insertSpaces=true tabSize=4