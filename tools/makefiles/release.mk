CPPFLAGS += -DNDEBUG -DBUILD_TYPE_RELEASE
CFLAGS += -Os -g3
CXXFLAGS += -Os -g3
ASFLAGS += -Os -g3
LDFLAGS += -Os -g3

# Optimised code: gas cannot interleave source (-h) once the compiler reorders lines
# ("unable to rebuffer file", issue #97), so every architecture lists without it; debug keeps -adhlns
LISTING_STYLE := -adlns
