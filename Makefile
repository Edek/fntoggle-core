# Compiler and flags
CC      := clang
CFLAGS  := -O2 -Wall -Wextra
LDFLAGS := -framework IOKit -framework CoreFoundation

# Binary and headers
TARGET  := fntoggle
SRC     := fntoggle.c
HEADERS := fntoggle.h

# Install location (override with: make install PREFIX=/custom/path)
PREFIX  ?= /usr/local
BINDIR  := $(PREFIX)/bin
INCDIR  := $(PREFIX)/include

# -------------------------
# Build
# -------------------------
all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# -------------------------
# Install binary
# -------------------------
install: $(TARGET)
	mkdir -p $(BINDIR)
	mv $(TARGET) $(BINDIR)/$(TARGET)
	chmod +x $(BINDIR)/$(TARGET)
	@echo "Installed to $(BINDIR)/$(TARGET)"

# -------------------------
# Install header (for library use)
# -------------------------
install-header: $(HEADERS)
	mkdir -p $(INCDIR)
	cp $(HEADERS) $(INCDIR)/$(HEADERS)
	@echo "Installed header to $(INCDIR)/$(HEADERS)"

# -------------------------
# Install everything
# -------------------------
install-all: install install-header

# -------------------------
# Uninstall
# -------------------------
uninstall:
	rm -f $(BINDIR)/$(TARGET)
	rm -f $(INCDIR)/$(HEADERS)
	@echo "Removed $(BINDIR)/$(TARGET) and $(INCDIR)/$(HEADERS)"

# -------------------------
# Clean
# -------------------------
clean:
	rm -f $(TARGET)

# -------------------------
# Rebuild
# -------------------------
re: clean all

.PHONY: all install install-header install-all uninstall clean re