add_subdirectory(snappy)
add_subdirectory(libiberty)
add_subdirectory(cleartextparser)
if( UNIX )
    add_subdirectory(cryptopp)
else()
    set_target_properties (PPthird_libiberty PROPERTIES FOLDER "Pinpoint third-party")
endif()

if (ENABLE_JEMALLOC)
    if( WIN32 AND NOT CYGWIN )
        add_subdirectory(jemalloc)
        set_target_properties (jemalloc PROPERTIES FOLDER "Pinpoint third-party")
        set_target_properties (jemallocso PROPERTIES FOLDER "Pinpoint third-party")
    else( WIN32 AND NOT CYGWIN )
        set(JEMALLOC_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/jemalloc/build)
        set(JEMALLOC_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/jemalloc)
        set(JEMALLOC_LIB ${JEMALLOC_BUILD_DIR}/lib/libjemalloc.a)

        if(EXISTS ${JEMALLOC_LIB})
            ADD_CUSTOM_COMMAND(
                OUTPUT jemalloc
                COMMAND cp ${JEMALLOC_LIB} ${PROJECT_BINARY_DIR}/lib/libPPJemalloc.a
            )
        else()
            file(MAKE_DIRECTORY ${JEMALLOC_BUILD_DIR})
            ADD_CUSTOM_COMMAND(
                OUTPUT jemalloc
                COMMAND cd ${JEMALLOC_SRC_DIR} && ./autogen.sh
                COMMAND cd ${JEMALLOC_BUILD_DIR} && ${JEMALLOC_SRC_DIR}/configure
                COMMAND cd ${JEMALLOC_BUILD_DIR} && make -j8
                COMMAND cp ${JEMALLOC_LIB} ${PROJECT_BINARY_DIR}/lib/libPPJemalloc.a
            )
        endif()

        add_custom_target(PPJemalloc ALL DEPENDS jemalloc)
    endif( WIN32 AND NOT CYGWIN )
endif()


