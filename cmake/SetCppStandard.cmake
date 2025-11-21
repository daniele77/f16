# -------------------------------------------------------------
# SetCppStandard.cmake
#
# Sets a default C++ standard and allows the user
# to override it from the command line.
# Requires the standard to be >= 17.
# -------------------------------------------------------------

# --- 1. Set a default if the user has not defined it ---
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to use (>= 17)" FORCE)
else()
    # Always insert the value into the cache (for GUIs like ccmake)
    set(CMAKE_CXX_STANDARD "${CMAKE_CXX_STANDARD}" CACHE STRING "C++ standard to use (>= 17)" FORCE)
endif()

# --- 2. Suggested values (for GUI and ccmake) ---
set_property(CACHE CMAKE_CXX_STANDARD PROPERTY STRINGS 17 20 23 26)

# --- 3. Validation ---
if (CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "CMAKE_CXX_STANDARD must be >= 17")
endif()

# --- 4. Recommended settings ---
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS OFF)
