cmake_minimum_required(VERSION 3.10)

project(kiwi8086)

add_executable(kiwi8086 "src/main.c" "include/system.h" "include/cpu.h" "src/system.c" "src/cpu/cpu_exec.c" "include/vga.h"   "src/cpu/modrm.c"  "src/cpu/regindex.c" "src/cpu/flags.c" "src/mem/segmem.c"  "include/8259pic.h")

find_package(termmanip REQUIRED)

target_link_libraries(kiwi8086 PUBLIC termmanip)

target_include_directories(kiwi8086 PUBLIC include)