
include_directories(BEFORE 
      ${PROJECT_SOURCE_DIR}/include
	  ${PROJECT_SOURCE_DIR}/third-party
	  ${PROJECT_SOURCE_DIR}/config)

SET (LIB_SRC 
cleartextParser.cpp
)

ADD_LIBRARY (cleartextParser STATIC ${LIB_SRC})
target_link_libraries (cleartextParser pcre_static)

GET_TARGET_PROPERTY (OUTPUT_VALUE cleartextParser OUTPUT_NAME)
MESSAGE (STATUS "This is the static cleartextParser library OUTPUT_NAME: " ${OUTPUT_VALUE})
SET_TARGET_PROPERTIES (cleartextParser PROPERTIES CLEAN_DIRECT_OUTPUT 1)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION .)
install(TARGETS cleartextParser DESTINATION lib)