CPPFLAGS += -DNDEBUG -DBUILD_TYPE_FAST
CFLAGS += -Os -g3
CXXFLAGS += -Os -g3
ASFLAGS += -Os -g3
LDFLAGS += -Os -g3

# The fast flavour (PLAN D162, D163): the speed flavour (D73) with a faster libm of relaxed accuracy, round to
# nearest only. LIBC_FLAVOURS lists the flavour directories in increasing priority, so a fast build takes the
# speed kernels, the speed/ files, then the fast/ files (libc/libc.mk, libc/math/subdir.mk)
export LIBC_FLAVOUR := fast
export LIBC_FLAVOURS := speed fast
LIBC_SPEED_CFLAGS := -O2

# Optimised code: gas cannot interleave source (-h) once the compiler reorders lines
# ("unable to rebuffer file", issue #97), so every architecture lists without it; debug keeps -adhlns
LISTING_STYLE := -adlns
