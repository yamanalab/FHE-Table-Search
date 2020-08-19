file(GLOB sources *.cpp)

set(name dec)
add_executable(${name} ${sources})

target_link_libraries(${name} fts_dec ${COMMON_LIBS})
