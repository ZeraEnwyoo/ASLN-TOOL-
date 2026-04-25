CC = gcc
CFLAGS = -Wall -O2 -pthread -D_DEFAULT_SOURCE -Iinclude
LIBS = -lcurl -lssl -lcrypto -ljpeg -lpng

TARGET = asln
SRCDIR = src
OBJDIR = obj

SOURCES = $(SRCDIR)/main.c $(SRCDIR)/dynamic_key.c $(SRCDIR)/exif_cleaner.c $(SRCDIR)/cache.c
OBJECTS = $(OBJDIR)/main.o $(OBJDIR)/dynamic_key.o $(OBJDIR)/exif_cleaner.o $(OBJDIR)/cache.o

all: $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LIBS)

clean:
	rm -rf $(OBJDIR) $(TARGET)

install:
	cp $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean install uninstall
