AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak
include $(AXIS_TOP_DIR)/tools/build/rules/recurse.mak

PROG    = analyze

OBJS    = analyze_error.o            analyze_util.o           main.o           motion_detection.o           storage.o

PKGS    = glib-2.0 axhttp axevent axparameter axstorage
CFLAGS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_LIBDIR) pkg-config --cflags $(PKGS))
LDLIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_LIBDIR) pkg-config --libs $(PKGS))
LDLIBS += -lcapture

CFLAGS += -g
CFLAGS += -W           -Wformat=2           -Wpointer-arith           -Wbad-function-cast           -Wstrict-prototypes           -Wmissing-prototypes           -Winline           -Wfloat-equal           -Wall           -Werror

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

clean:
	$(RM) $(OBJS) $(PROG)
