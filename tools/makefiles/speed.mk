CPPFLAGS += -DNDEBUG -DBUILD_TYPE_SPEED
CFLAGS += -Os -g3
CXXFLAGS += -Os -g3
ASFLAGS += -Os -g3
LDFLAGS += -Os -g3

# The speed flavour (PLAN D12, D73): -Os like release (a whole -O2 library breaks the
# +10 % size budget), with the arch files' speed kernels; a hot module may add
# LIBC_SPEED_CFLAGS to its own CFLAGS
export LIBC_FLAVOUR := speed
LIBC_SPEED_CFLAGS := -O2

# Optimised code: gas cannot interleave source (-h) once the compiler reorders lines
# ("unable to rebuffer file", issue #97), so every architecture lists without it; debug keeps -adhlns
LISTING_STYLE := -adlns
