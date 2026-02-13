set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# IMPORTANT: use host-installed cross compiler, NOT sysroot/bin
set(CMAKE_C_COMPILER   /usr/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabihf-g++)

set(CMAKE_SYSROOT //rpi-env/rpi-sysroot)
set(CMAKE_FIND_ROOT_PATH //rpi-env/rpi-sysroot)

# Search headers/libs in sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Optional: help find /usr/local in sysroot
list(APPEND CMAKE_PREFIX_PATH
  //rpi-env/rpi-sysroot/usr/local
  //rpi-env/rpi-sysroot/usr
)
