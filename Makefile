TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
CFLAGS=-Iinclude

run: clean default
	[ -f ./mynewdb.db ] || ./$(TARGET) -nfmynewdb.db

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	# rm -f *.db

$(TARGET): $(OBJ)
	mkdir -p bin
	cc -o $@ $?

obj/%.o: src/%.c
	mkdir -p obj
	cc -c $< -o $@ $(CFLAGS)
