IDIR =include# directory for header files
SDIR =src# source directory
ODIR =obj# object file directory
CC = gcc# compiler used
CFLAGS = -std=c99 -Wall -Wshadow -Werror -I$(IDIR)# compiler flags
LDFLAGS =# -lm library flags
TARGET = shell # file executable generated

# Getting the list of all c and object files
SOURCES := $(wildcard $(SDIR)/*.c)
OBJECTS := $(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(SOURCES))

# TO manually list the header files
# _DEPS = common.h # dependency header files

# Getting the list of header files
HEADERS = $(wildcard $(IDIR)/*.h)

# To print the value of a variable
# $(info    VAR is $(HEADERS))

# Pattern substitution
# $(patsubst pattern, substitution, text_to_insert)
# DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

all : $(TARGET)

$(ODIR)/%.o: $(SDIR)/%.c $(HEADERS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

# Any file with the name clean will not interrupt the cmd clean
.PHONY: clean

clean:
	rm -rf $(ODIR) *.DS_Store $(TARGET) 
