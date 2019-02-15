IDIR =./include# directory for header files
SDIR =./src# source directory
CC = gcc# compiler used
CFLAGS = -Wall -Wshadow -Werror -I$(IDIR)# compiler flags
LDFLAGS =# -lm library flags
ODIR = obj# object file directory

# Listing the header and object files
_DEPS = builtin_cmd_handler.h# dependency header files
_OBJ = main.o builtin_cmd_handler.o# object files
_SRC = main.c builtin_cmd_handler.c# src c files

# Pattern substitution
# $(patsubst pattern, substitution, text)
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
SRC = $(patsubst %,$(SDIR)/%,$(_SRC))

TARGET = shell # file executable generated

all : $(TARGET)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

# Any file with the name clean will not interrupt the cmd clean
.PHONY: clean

clean:
	rm -rf $(ODIR)/*.o *.DS_Storema
