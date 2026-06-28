userd: client daemon

debug:
	@mkdir -p bin
	@g++ -g -DDEBUG src/client/main.cpp -o bin/userctl -Isrc/client -Wall -Wextra
	@g++ -g -DDEBUG src/server/main.cpp -o bin/userd -Isrc/server -Wall -Wextra

client: src/client/main.cpp
	@mkdir -p bin
	@g++ -O2 src/client/main.cpp -o bin/userctl -Isrc/client -Wall -Wextra

daemon: src/server/main.cpp
	@mkdir -p bin
	@g++ -O2 src/server/main.cpp -o bin/userd -Isrc/server -Wall -Wextra

install: client daemon
	@mkdir -p /usr/local/bin
	@cp bin/userctl /usr/local/bin/
	@cp bin/userd /usr/local/bin/