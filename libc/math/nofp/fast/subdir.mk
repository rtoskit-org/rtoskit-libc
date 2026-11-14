# The layer's fast files, less those a later layer of the core provides, plain, in speed/ or in fast/ (PLAN D168)
NOFP_FAST_LATER := $(call libc_math_layer_stems,$(call libc_math_later,nofp))
SRC := $(filter-out $(foreach stem,${NOFP_FAST_LATER},${SOURCE_DIR}/${stem}.c ${SOURCE_DIR}/${stem}.S),$(wildcard ${SOURCE_DIR}/*.c ${SOURCE_DIR}/*.S))
