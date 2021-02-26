AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak

PROG    = ioinfo

PKGS    = glib-2.0 gobject-2.0
CFLAGS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_LIBDIR) pkg-config --cflags $(PKGS))
LDLIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_LIBDIR) pkg-config --libs $(PKGS))
LDLIBS += -laxparameter

CFLAGS += -W           -Wformat=2           -Wpointer-arith           -Wbad-function-cast           -Wstrict-prototypes           -Wmissing-prototypes           -Winline           -Wfloat-equal           -Wall           -Werror

all: $(PROG)

clean:
	$(RM) *.o $(PROG)

print-%:
	@echo '$*=$($*)'
