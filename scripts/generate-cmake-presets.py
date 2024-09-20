import common
import argparse
from typing import List, Tuple

class CMakePresetsGenerator:
    # Static dictionary to map graphic API strings.
    __graphic_api_to_str = {
        "opengl": "OpenGL",
        "metal": "Metal",
        "vulkan": "Vulkan",
        "directx": "DirectX"
    }

    def __init__(self, arch: str, compiler: str, gapi: str) -> None:
        common.detect_git()
        common.detect_cmake()

        self.arch = arch
        print(f"Selected architecture: {self.arch}")

        self.gapi = gapi
        print(f"Selected graphic API: {self.gapi}")

        self.os = common.detect_os()
        print(f"Detected operating system: {self.os}")

        self.ninja = common.detect_ninja()
        print(f"Ninja found at: {self.ninja}")

        self.clang = common.detect_c_compiler(compiler)
        print(f"Clang found at: {self.clang}")

        self.clangxx = common.detect_cxx_compiler(compiler)
        print(f"Clang++ found at: {self.clangxx}")

        self.project_root = common.get_project_root()
        print(f"Project root directory: {self.project_root}")

        self.osx_variables = ""

    def __get_supported_archs(self) -> List[str]:
        if "native" == self.arch:
            if "MacOSX" == self.os:
                return ["arm64", "x86_64"]
            else:
                return ["x86_64"]

        if "arm64" == self.arch and "MacOSX" != self.os:
            raise ValueError(f"{self.arch} isn't supported on {self.os}.")

        return [self.arch]

    def __get_supported_gapis(self) -> List[Tuple[str, str]]:
        # Currently only OpenGL is supported.
        graphic_api = self.__graphic_api_to_str[self.gapi]
        if "OpenGL" != graphic_api:
            raise ValueError(f"{graphic_api} is not supported. Only OpenGL is currently supported.")

        return [(graphic_api, "")]

    def __generate_config_presets(self) -> str:
        config_presets_json = ""
        # Generate config presets for each supported architecture...
        for arch in self.__get_supported_archs():
            if "MacOSX" == self.os:
                osx_min_version = common.apple_minimum_supported_version("macosx")
                osx_sdk_path = common.xcrun_get_sdk_path("macosx")
                self.osx_variables = f"""\
                "CMAKE_OSX_ARCHITECTURES":      "{arch.lower()}",
                "CMAKE_OSX_DEPLOYMENT_TARGET":  "{osx_min_version}",
                "CMAKE_OSX_SYSROOT":            "{osx_sdk_path}",\
                """.lstrip()

            # ... as well as Graphic APIs supported by this platform
            for gapi, desc in self.__get_supported_gapis():
                config_presets_json += f"""\
        {{
            "name":         "{self.os.lower()}-{arch.lower()}-{gapi.lower()}-debug",
            "displayName":  "{self.os.lower()}-{arch.lower()}-{gapi.lower()}-debug",
            "description":  "{desc}",
            "inherits":     "base",
            "architecture": {{
                "value":    "{arch.lower()}",
                "strategy": "external"
            }},
            "cacheVariables": {{
                "CMAKE_MAKE_PROGRAM":           "{self.ninja}",
                "CMAKE_BUILD_TYPE":             "Debug",
                "CMAKE_CXX_COMPILER":           "{self.clangxx}",
                "CMAKE_C_COMPILER":             "{self.clang}",
                "CMAKE_OBJCXX_COMPILER":        "{self.clangxx}",
                "CMAKE_OBJC_COMPILER":          "{self.clang}",
                {self.osx_variables}
                "FLUX_GRAPHICS_API":            "{gapi}"
            }}
        }},
        {{
            "name":         "{self.os.lower()}-{arch.lower()}-{gapi.lower()}-release",
            "displayName":  "{self.os.lower()}-{arch.lower()}-{gapi.lower()}-release",
            "description":  "{desc}",
            "inherits":     "base",
            "architecture": {{
                "value":    "{arch.lower()}",
                "strategy": "external"
            }},
            "cacheVariables": {{
                "CMAKE_MAKE_PROGRAM":           "{self.ninja}",
                "CMAKE_BUILD_TYPE":             "RelWithDebInfo",
                "CMAKE_CXX_COMPILER":           "{self.clangxx}",
                "CMAKE_C_COMPILER":             "{self.clang}",
                "CMAKE_OBJCXX_COMPILER":        "{self.clangxx}",
                "CMAKE_OBJC_COMPILER":          "{self.clang}",
                {self.osx_variables}
                "FLUX_GRAPHICS_API":            "{gapi}"
            }}
        }},\n"""

        return config_presets_json

    def generate_cmake_presets(self) -> None:
        with open(f"{self.project_root}/CMakePresets.json", "w") as cmake_presets:
            cmake_presets.write(f"""\
{{
    "version": 3,
    "configurePresets": [
{self.__generate_config_presets().rstrip()}
        {{
            "name":         "base",
            "description":  "For more information: http://aka.ms/cmakepresetsvs",
            "hidden":       true,
            "generator":    "Ninja",
            "binaryDir":    "${{sourceDir}}/build/${{presetName}}",
            "installDir":   "${{sourceDir}}/build/${{presetName}}/output/"
        }}
    ]
}}""")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="CMakePresetsGenerator",
                                     description="Script for generating CMakePresets file.")
    parser.add_argument("--arch", choices=["native", "x86_64", "arm64"], default="native",
                        help="supported architectures (default: %(default)s)")
    parser.add_argument("--compiler", choices=["clang", "gcc", "msvc"], default="clang",
                        help="gcc and msvc compilers are not currently supported (default: %(default)s)")
    parser.add_argument("--gapi", choices=["opengl", "metal", "vulkan", "directx"], default="opengl",
                        help="metal, vulkan and directx graphic APIs are not currently supported (default: %(default)s)")
    args = parser.parse_args()
    try:
        presets_generator = CMakePresetsGenerator(args.arch, args.compiler, args.gapi)
        presets_generator.generate_cmake_presets()
    except Exception as e:
        print(e)