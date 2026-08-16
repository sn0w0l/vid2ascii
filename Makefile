# ==============================================================================
#  Project layout
#    src/      – all .c source files
#    include/  – all .h headers
#    build/    – object files + dependency files
#    tests/    – test sources, compiled separately
# ==============================================================================

CC      := gcc
TARGET  := vid2ascii

SRCDIR  := src
INCDIR  := include
BUILDDIR:= build

SRCS    := $(wildcard $(SRCDIR)/*.c)
OBJS    := $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))
DEPS    := $(OBJS:.o=.d)

WARNINGS := \
    -Wall -Wextra -Wpedantic          \
    -Wshadow -Wcast-align             \
    -Wwrite-strings -Wmissing-prototypes \
    -Wstrict-prototypes -Wdouble-promotion \
    -Wformat=2 -Wnull-dereference     \
    -Wimplicit-fallthrough            \
    -Wconversion -Wsign-conversion

CFLAGS  := -std=gnu11 $(WARNINGS) -I$(INCDIR)
LDFLAGS :=
LDLIBS  :=

#  == BUILD MODE ==
#  Default: dev
#  make DEBUG=1     →  symbols + sanitisers, -O0
#  make RELEASE=1   →  full optimisation, -DNDEBUG, LTO
ifeq ($(DEBUG),1)
    CFLAGS  += -g3 -O0 -DDEBUG \
               -fsanitize=address,undefined \
               -fno-omit-frame-pointer
    LDFLAGS += -fsanitize=address,undefined
    BUILD_TAG := [DEBUG]
else ifeq ($(RELEASE),1)
    CFLAGS  += -O3 -march=native -DNDEBUG -flto
    LDFLAGS += -flto
    BUILD_TAG := [RELEASE]
else
    CFLAGS  += -Og -g
    BUILD_TAG := [DEV]
endif

ifeq ($(IMPL_BITMAP),1)
	CFLAGS += -DIMPL_BITMAP
endif

DEPFLAGS = -MMD -MP

.PHONY: all clean test info

all: $(TARGET)
	@echo "$(BUILD_TAG)  Built $(TARGET)"

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@


$(BUILDDIR):
	mkdir -p $(BUILDDIR)

-include $(DEPS)

clean:
	$(RM) -r $(BUILDDIR) $(TARGET)

info:
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "SRCS    = $(SRCS)"
	@echo "OBJS    = $(OBJS)"
