HEADERS = ../*.h  ../*/*.h ../*/*/*.h
OBJECTS = ./src/main.o ./src/json.o
PROG = btch

CC=gcc 
CFLAGS= -Wall -Wextra -g -lcurl

GREEN=\033[0;32m
NC =\033[0m

.PHONY: clean all default build run debug install uninstall depends

default: build

%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@ $(CFLAGS)

build: $(OBJECTS)
	$(CC) $(OBJECTS) -o ./bin/$@.exe $(CFLAGS)
	@echo "$(GREEN)Built$(NC)"

install: build
	sudo mv ./bin/build.exe /usr/local/bin/$(PROG)
	@echo "$(GREEN)Installed$(NC)"
	@btch -h

depends:
	brew install curl
	@echo "$(GREEN)Dependencies Installed"

uninstall:
	-sudo rm -f /usr/local/bin/$(PROG)
	-rm -f $(HOME)/.local/share/openai_key.txt
	@echo "$(GREEN)Uninstalled$(NC)"
clean:
	-rm -f $(OBJECTS)
	-rm -f ./bin/build.exe