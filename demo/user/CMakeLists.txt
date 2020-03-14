file(GLOB sources *.cpp)

set(name user)
add_executable(${name} ${sources})

target_link_libraries(${name} fts_user ${COMMON_LIBS})
