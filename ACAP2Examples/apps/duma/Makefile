AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/Rules.axis

PROGS     = example1 example2 example3 example4 example5

INSTDIR   = $(prefix)/bin/
INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root


# Add the duma lib static
LDFLAGS += -lpthread
LDLIBS   += $(prefix)/lib/libduma.a


CFLAGS += -Wall -g -O2

all:	$(PROGS)

example1: example1.o
	$(CC) $(LDFLAGS) $^ $(LIBS) $(LDLIBS) -o $@

example2: example2.o
	$(CC) $(LDFLAGS) $^ $(LIBS) $(LDLIBS) -o $@

example3: example3.o
	$(CC) $(LDFLAGS) $^ $(LIBS) $(LDLIBS) -o $@

example4: example4.o
	$(CC) $(LDFLAGS) $^ $(LIBS) $(LDLIBS) -o $@

example5: example5.o
	$(CC) $(LDFLAGS) $^ $(LIBS) $(LDLIBS) -o $@

clean:
	rm -f $(PROGS) *.o core

