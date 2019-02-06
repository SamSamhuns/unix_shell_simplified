CC = gcc # compiler
CFLAGS = -Wall -Werror -Wshadow # compiler flags
LDFLAGS = #-lm # libraries used in compile time
OBJFILES = cmd_handler.o main.o # object files
TARGET = shell # final executable file

# to build executable
all: $(TARGET)

# To clean executables
clean:
	rm -rf $(OBJFILES) $(TARGET) *~

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

# build cmd_handler.o obj file
cmd_handler.o: cmd_handler.c cmd_handler.h
	$(CC) $(CFLAGS) -c cmd_handler.c

# build main.o obj file
main.o: main.c
	$(CC) $(CFLAGS) -c main.c
