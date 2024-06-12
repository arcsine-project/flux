flux_static_library(foundation
    COMMON
        TEST
            "flux/foundation/memory/detail/constexpr_memcpy-test.cpp"
            "flux/foundation/memory/detail/debug_helpers-test.cpp"
            "flux/foundation/memory/detail/fixed_stack-test.cpp"
            "flux/foundation/memory/detail/free_list_array-test.cpp"
            "flux/foundation/memory/detail/free_list-test.cpp"
            "flux/foundation/memory/align-test.cpp"
            "flux/foundation/memory/allocator_storage-test.cpp"
            "flux/foundation/memory/construct-test.cpp"
            "flux/foundation/memory/deleter-test.cpp"
            "flux/foundation/memory/heap_allocator-test.cpp"
            "flux/foundation/memory/memory_arena-test.cpp"
            "flux/foundation/memory/memory_block-test.cpp"
            "flux/foundation/memory/memory_pool-test.cpp"
            "flux/foundation/memory/memory_pool_list-test.cpp"
            "flux/foundation/memory/memory_stack-test.cpp"
            "flux/foundation/memory/relocate-test.cpp"
            "flux/foundation/memory/static_allocator-test.cpp"
            "flux/foundation/memory/std_allocator_adapter-test.cpp"
            "flux/foundation/memory/temporary_allocator-test.cpp"
            "flux/foundation/memory/threading-test.cpp"
            "flux/foundation/memory/uninitialized_algorithms-test.cpp"
            "flux/foundation/memory/uninitialized_storage-test.cpp"

            "flux/foundation/utility/iterator_adapter-test.cpp"

            "flux/foundation/containers/static_vector-test.cpp"
            "flux/foundation/containers/vector-test.cpp"
        SOURCE
            "flux/foundation/memory/detail/debug_helpers.cpp"
            "flux/foundation/memory/debugging.cpp"
            "flux/foundation/memory/temporary_allocator.cpp"
        LINK
            flux::io
            flux::platform)

# code: language="CMake" insertSpaces=true tabSize=4