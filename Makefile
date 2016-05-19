C_FILES := $(wildcard *.c)
OBJ_FILES := $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
CC_FLAGS := -Wall -Werror -ansi -pedantic

all: prog02

prog02: $(OBJ_FILES)
	mkdir -p bin
	gcc -o bin/$@ $^

obj/%.o: src/%.cpp
	mkdir -p obj
	gcc $(CC_FLAGS) -c -o $@ $<

debug: $(OBJ_FILES)
	gcc -g $(CC_FLAGS) -o $@ $^

clndbg:
	rm debug
	rm -r obj

clean:
	rm -r bin
	rm -r obj
