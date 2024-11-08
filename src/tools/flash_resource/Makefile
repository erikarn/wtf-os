
OBJS=make_entry.o
CFLAGS=-O2 -Wall -Werror

all: default

default: make_entry

make_entry: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o make_entry

clean:
	rm -f $(OBJS) make_entry

