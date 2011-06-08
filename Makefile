CFLAGS = -Wall -g -DDEBUG
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

all: test

%.o : %.c
		@echo " > Compiling $< .."
		@$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

test: $(OBJ)
		@echo " > Linking $@ .."
		@$(CC) $(LDFLAGS) $(OBJECTS) $(LIBDIRS) $(LIBS) -o $@ $^

clean: 
		@echo " > Cleaning .."
		@rm -f $(OBJ) test
