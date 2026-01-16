#
# Relative path to the top of the source tree.
#
LEVEL=..

#
# List all of the subdirectories that we will compile.
#
# Do not use PARALLEL_DIRS, most tools here depend on libz3,
# which may lead to multiple building processes for the building 
# target z3 (see in $(LEVEL)/Makefile.common.in)
# and finally cause "file truncated" issue. 
DIRS = $(patsubst $(PROJ_SRC_DIR)/%/Makefile,%,$(wildcard $(PROJ_SRC_DIR)/*/Makefile))

include $(LEVEL)/Makefile.common
