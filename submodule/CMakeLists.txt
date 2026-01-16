

#add_custom_command(
#    OUTPUT tmp
#    COMMAND
#    scons -C "${CMAKE_CURRENT_SOURCE_DIR}/canary" libonly=1 lib=DyckAA lib=DyckGraph lib=DyckCG
#    COMMENT "building canary"
#    )
#add_custom_target(canar ALL
#    DEPENDS tmp
#    )
add_subdirectory(SMT)
set_target_properties (SMT PROPERTIES FOLDER "SMT")