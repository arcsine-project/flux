import sys
import subprocess
import re
import shutil
import glob
import os
from typing import Optional, Tuple

# Returns the absolute path to the root directory of the project.
def get_project_root() -> str:
    # Get the directory of the current script
    script_dir = os.path.dirname(os.path.realpath(__file__))

    # Return the absolute path to the project root
    project_root = os.path.abspath(os.path.join(script_dir, ".."))
    return project_root

def apple_minimum_supported_version(os: str) -> Optional[int]:
    if os == "macosx":
        return 11
    elif os == "iphoneos":
        return 13
    elif os == "iphonesimulator":
        return 13
    return None

def xcrun_get_sdk_path(os: str) -> str:
    try:
        # Run the xcrun command with the specified SDK and capture the output
        result = subprocess.run(["xcrun", "--sdk", os, "--show-sdk-path"], 
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        # Check if the command was successful
        if result.returncode == 0:
            return result.stdout.strip()
        else:
            raise RuntimeError(f"Error running xcrun: {result.stderr.strip()}")
    except FileNotFoundError:
        raise FileNotFoundError("xcrun command not found. Make sure Xcode command line tools are installed.")
    except Exception as e:
        raise RuntimeError(f"An error occurred while running xcrun: {str(e)}")

def detect_os() -> str:
    if sys.platform.startswith("win32"):
        return "Windows"
    elif sys.platform.startswith("darwin"):
        return "MacOSX"
    elif sys.platform.startswith("linux"):
        return "Linux"
    else:
        return "Unknown"

def detect_git() -> str:
    git_path = shutil.which("git")
    if not git_path:
        raise FileNotFoundError("Cannot find Git. Please make sure it is available in your PATH.")

    return git_path

def detect_ninja() -> str:
    ninja_path = shutil.which("ninja")
    if not ninja_path:
        raise FileNotFoundError("Cannot find Ninja. Please make sure it is available in your PATH.")

    return ninja_path

def detect_cmake() -> str:
    cmake_path = shutil.which("cmake")
    if not cmake_path:
        raise FileNotFoundError("Cannot find CMake. Please make sure it is available in your PATH.")

    return cmake_path

def detect_macports_clang(clang_name: str) -> Tuple[Optional[str], str]:
    macports_clang_paths = glob.glob(f"/opt/local/bin/{clang_name}-mp-*")
    if not macports_clang_paths:
        return None, "No MacPorts clang installations found."

    for clang_path in sorted(macports_clang_paths, reverse=True):
        try:
            result = subprocess.run([clang_path, "--version"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            if result.returncode != 0:
                continue

            version_info = result.stdout
            # Extract version number using regex
            version_match = re.search(r"clang version (\d+)\.(\d+)\.(\d+)", version_info)
            if version_match:
                macports_clang_version_major = int(version_match.group(1))
                if macports_clang_version_major >= 18:
                    return clang_path, f"MacPorts clang found at: {clang_path}"
        except Exception:
            continue

    return None, "No suitable MacPorts clang installation found with version 18 or greater."

def detect_default_clang(clang_name: str) -> Tuple[Optional[str], str]:
    clang_path = shutil.which(clang_name)
    if not clang_path:
        return None, f"Cannot find '{clang_name}'. Please make sure it is available in your PATH."
    
    result = subprocess.run([clang_name, "--version"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if result.returncode != 0:
        return None, f"Failed to run '{clang_name} --version'."
    
    version_info = result.stdout
    # Extract version number using regex
    version_match = re.search(r"clang version (\d+)\.(\d+)\.(\d+)", version_info)
    if version_match:
        clang_version_major = int(version_match.group(1))
        if clang_version_major >= 18:
            return clang_path, f"clang found at: {clang_path}"
        else:
            return None, f"'{clang_name}' version must be 18 or greater. Found version {clang_version_major}."
    else:
        return None, f"Could not determine the version of '{clang_name}'."

def detect_cxx_compiler(name: str) -> str:
    if name not in ["clang", "clang++"]:
        raise ValueError("Currently only 'clang' and 'clang++' are supported as compiler names.")
    
    default_clang_path, default_message = detect_default_clang(name)
    if default_clang_path:
        return default_clang_path
    elif detect_os() == "MacOSX":
        macports_clang_path, macports_message = detect_macports_clang(name)
        if macports_clang_path:
            return macports_clang_path
        else:
            raise RuntimeError(f"An error occurred: {default_message} {macports_message}")
    else:
        raise RuntimeError(f"An error occurred: {default_message}")

# if __name__ == "__main__":
#     detected_os = detect_os()
#     print(f"The current operating system is: {detected_os}")

#     ninja = detect_ninja()
#     print(f"Ninja found at: {ninja}")

#     project_root = get_project_root()
#     print(f"Project root directory is: {project_root}")
    
#     try:
#         compiler_path = detect_cxx_compiler("clang")
#         print(f"Clang compiler found at: {compiler_path}")
#     except Exception as e:
#         print(e)

#     try:
#         compiler_path = detect_cxx_compiler("clang++")
#         print(f"Clang++ compiler found at: {compiler_path}")
#     except Exception as e:
#         print(e)