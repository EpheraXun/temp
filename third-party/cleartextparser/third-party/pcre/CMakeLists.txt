
file(GLOB SRC "*.c")
file(GLOB HEADER "*.h")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
ADD_DEFINITIONS(-DHAVE_CONFIG_H)

add_library(pcre_static STATIC ${SRC})

set_target_properties(pcre_static PROPERTIES OUTPUT_NAME pcre)
target_include_directories (pcre_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS pcre_static DESTINATION lib)
install(FILES ${HEADER} DESTINATION include/pcre)