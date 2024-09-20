find_package(fast_io REQUIRED)

flux_interface_library(io
    COMMON
        TEST
            "flux/io/manipulators-test.cpp"
            "flux/io/print-test.cpp"
        LINK
            fast_io::fast_io
            flux::meta)

# code: language="CMake" insertSpaces=true tabSize=4