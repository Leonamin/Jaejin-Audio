CC=gcc
CFLAGS=-g

LDFLAGS=
LDLIBS=-lasound

BUILDDIR=build/
SRCDIR=src/

OBJS=$(BUILDDIR)main.o
SRCS=$(SRCDIR)main.c
TARGET=$(BUILDDIR)app.out
 
all: $(TARGET)
 
clean:
	rm -f *.o
	rm -f $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

$(OBJS) : $(SRCS)
	$(CC) -c -o $(OBJS) $(SRCS)