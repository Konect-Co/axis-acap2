AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak

SRC = main
OBJS = $(SRC).o
BIN = app_with_shared_lib

# Shared-lib specific
SHLIB_NAME    = lib_shared
SHLIB         = $(SHLIB_NAME).so
SHLIB_DIR     = ./lib
SHLIB_FLAGS   = -L$(SHLIB_DIR) -Wl,-R,'$$ORIGIN/lib'
SHLIB_SRC     = $(SHLIB_NAME).c
SHLIB_OBJ     = $(SHLIB_NAME).o

CFLAGS  +=  $(SHLIB_FLAGS)
LDFLAGS += -l_shared

all: $(SRC)

make-target:
	$(MAKE) $(TARGET_NAME)

$(SHLIB):
	$(CC) -fPIC -c $(SHLIB_SRC)
	$(LD) -shared -o $(SHLIB) $(SHLIB_OBJ)
	# Copy the shared lib to lib folder in order to embed it into the .eap file
	@mkdir -vp $(SHLIB_DIR)
	@cp -vf $(SHLIB) $(SHLIB_DIR)

$(SRC) : $(SHLIB) $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $(BIN)

clean:
	rm -f $(BIN) *.o *.eap *.eap.old *.so lib/* *.orig *~

