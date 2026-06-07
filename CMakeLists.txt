cmake_minimum_required(VERSION 3.10)
project(RayTracer)

set(CMAKE_CXX_STANDARD 17)

# --- SPEED HACKS FOR VISUAL STUDIO (MSVC) ---
if(MSVC)
    add_compile_options(
        $<$<NOT:$<CONFIG:Debug>>:/Ox>
        $<$<NOT:$<CONFIG:Debug>>:/Ot>
        $<$<NOT:$<CONFIG:Debug>>:/fp:fast>
        $<$<NOT:$<CONFIG:Debug>>:/arch:AVX2>
        $<$<NOT:$<CONFIG:Debug>>:/GL>
        /MP # This one is safe for both (compiles faster)
    )
endif()

add_executable(inOneWeekend main.cc)