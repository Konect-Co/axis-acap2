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

PROGS     = licensekey
CFLAGS   += -Wall -g -O2
LDLIBS   += -Wl,-Bstatic,-llicensekey_stat,-Bdynamic,-llicensekey -ldl

# Strip the binaries when building unless debug or static
LDFLAGS  += -s
SRCS      = licensekey.c
OBJS      = $(SRCS:.c=.o)

ifeq ($(AXIS_BUILDTYPE), host)
all:
	@echo "you must define AXIS_BUILDTYPE"
	exit 1

else
all:	$(PROGS)

endif

$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) $(LDLIBS) -o $@
	$(STRIP) $@

install:	$(PROGS)
	create-package.sh
	install-onto-target.sh

clean:
	rm -f $(PROGS) *.o core *.tar
