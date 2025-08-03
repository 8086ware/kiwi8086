cmake_minimum_required(VERSION 3.10)

project(kiwi8086)

add_executable(kiwi8086 "src/main.c" "include/system.h" "include/cpu/cpu.h" "src/system.c" "src/cpu/cpu_exec.c" "src/cpu/modrm.c"  "src/cpu/regindex.c" "src/cpu/flags.c" "src/mem/segmem.c" "include/pic.h" "src/cpu/instructions.c" "include/cpu/instructions.h" "include/memory.h" "src/mem/readandwrite.c" "include/display/mda.h" "include/display/crtc.h" "src/ps2/controller.c" "include/ps2/controller.h" "src/ps2/keyboard.c" "include/ps2/keyboard.h" "src/pic/handle_port.c")

target_link_options(kiwi8086 PUBLIC -fsanitize=address)

find_package(SDL3 REQUIRED)

target_link_libraries(kiwi8086 PUBLIC SDL3::SDL3)

target_include_directories(kiwi8086 PUBLIC include)

