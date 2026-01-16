set(LLVM_LINK_COMPONENTS2 option)
llvm_map_components_to_libnames(llvm_libs2 ${LLVM_LINK_COMPONENTS2})
aux_source_directory(. smtd_src)
add_executable(pp-smtd ${smtd_src})
set_target_properties (pp-smtd PROPERTIES FOLDER "Pinpoint executables")

add_dependencies(pp-smtd z3)
TARGET_LINK_LIBRARIES(pp-smtd SMT PPPerfUtil SMTSupport ${llvm_libs2} ${Z3LinkOption})
TARGET_LINK_LIBRARIES(pp-smtd ${PROJECT_BINARY_DIR}/lib/${Z3LibraryName})
