CC := gcc
CFLAGS := -Iinclude -Wall -Wextra -g
SRCDIR := src
BUILDDIR := build
TARGET := $(BUILDDIR)/project

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	$(TARGET)

clean:
	rm -rf $(BUILDDIR) $(SRCDIR)/*.o
