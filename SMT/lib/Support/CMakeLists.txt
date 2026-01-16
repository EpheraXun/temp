aux_source_directory(. SMTSupport_src)
add_library(SMTSupport STATIC ${SMTSupport_src})
target_link_libraries(SMTSupport
PPPerfUtil
)
set_target_properties (SMTSupport PROPERTIES FOLDER "SMT")