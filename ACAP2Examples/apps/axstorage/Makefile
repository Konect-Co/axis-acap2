# Note: the following 2 lines will use the Axis build system,
# which sets up all the $(CC) macros.
# To cross compile for a camera or video encoder,
# you must define  AXIS_BUILDTYPE
# This can be done on in 3 ways:
# - on the command line calling make
# - in this file before including the rules/common.mak
# - in the hidden file .target-makefrag (if it exists)
#
# use create-package.sh to generate a .target-makefrag (if it does not exist)
## AXIS_BUILDTYPE ?=

AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak

PROGS     = axStorageExample

CFLAGS   += -Wall -g -O2 -Werror

PKGS = glib-2.0 gio-2.0 axstorage
CFLAGS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_LIBDIR) pkg-config --cflags $(PKGS))
LDLIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_LIBDIR) pkg-config --libs $(PKGS))

SRCS      = ax_storage_example.c
OBJS      = $(SRCS:.c=.o)

# Add libraries here
ifeq ($(AXIS_OPT_DEBUG),y)
CFLAGS	+= -DDEBUG
endif

ifeq ($(AXIS_BUILDTYPE), host)
all:
	@echo "you must define AXIS_BUILDTYPE"
	exit 1

else
all:	$(PROGS)

endif

# Link and strip the binary
$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) $(LDLIBS) -o $@
	$(STRIP) $@

# Install the package on the camera
install:	$(PROGS)
#	@$(AXIS_TOP_DIR)/tools/scripts/create-package.sh
#	@$(AXIS_TOP_DIR)/tools/scripts/install-onto-target.sh install

clean:
	rm -f $(PROGS) *.o *.a core

