IDIR =include# directory for header files
SDIR =src# source directory
ODIR =obj# object file directory
CC = gcc# compiler used
CFLAGS = -Wall -Wshadow -Werror -I$(IDIR)# compiler flags
LDFLAGS =# -lm library flags
TARGET = shell # file executable generated

# Getting the list of all c and object files
SOURCES := $(wildcard $(SDIR)/*.c)
OBJECTS := $(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(SOURCES))

# Listing the header files
_DEPS = builtin_cmd_handler.h common.h # dependency header files

# Pattern substitution
# $(patsubst pattern, substitution, text_to_insert)
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

all : $(TARGET)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

# Any file with the name clean will not interrupt the cmd clean
.PHONY: clean

clean:
	rm -rf $(ODIR)/*.o *.DS_Store
