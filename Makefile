HEADERS = ../*.h  ../*/*.h ../*/*/*.h
OBJECTS = ./src/main.o ./src/json.o
PROG = btch

CC=gcc 
CFLAGS= -Wall -Wextra -g -lcurl

.PHONY: clean all default build run debug install uninstall depends

default: build

%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@ $(CFLAGS)

build: $(OBJECTS)
	$(CC) $(OBJECTS) -o ./bin/$@.exe $(CFLAGS)

install: build
	sudo mv ./bin/build.exe /usr/local/bin/$(PROG)

depends:
	brew install curl

uninstall:
	-sudo rm -f /usr/local/bin/$(PROG)
	-rm -f $(HOME)/.local/share/openai_key.txt
clean:
	-rm -f $(OBJECTS)
	-rm -f ./bin/build.exe