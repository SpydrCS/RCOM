# Makefile to build the project
# NOTE: This file must not be changed.

# Parameters
CC = gcc
CFLAGS = -Wall

SRC = src/
INCLUDE = include/
BIN = bin/

# ftp://[<user>:<password>@]<host>/<url-path>
FTP_KODI1 = ftp://ftp.up.pt/pub/kodi/timestamp.txt
FTP_KODI2 = ftp://anonymous:qualquer-password@ftp.up.pt/pub/kodi/robots.txt
FTP_KODI3 = ftp://anonymous:qualquer-password@ftp.up.pt/pub/kodi/screenshots/kodi-addons.jpg

FTP_NETLAB1 = ftp://rcom:rcom@netlab1.fe.up.pt/files/crab.mp4
FTP_NETLAB2 = ftp://rcom:rcom@netlab1.fe.up.pt/files/pic1.jpg
FTP_NETLAB3 = ftp://rcom:rcom@netlab1.fe.up.pt/pipe.txt

# Targets
.PHONY: all
all: $(BIN)/main

$(BIN)/main: main.c $(SRC)/*.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(INCLUDE)
 
.PHONY: clean
clean:
	rm -f $(BIN)/main

.PHONY: run_kodi1
run_kodi1:
	make clean && make && $(BIN)/main $(FTP_KODI1)

.PHONY: run_kodi2
run_kodi2:
	make clean && make && $(BIN)/main $(FTP_KODI2)

.PHONY: run_kodi3
run_kodiapk:
	make clean && make && $(BIN)/main $(FTP_KODI3)

.PHONY: run_netlab1
run_netlab:
	make clean && make && $(BIN)/main $(FTP_NETLAB1)

.PHONY: run_netlab2
run_netlab:
	make clean && make && $(BIN)/main $(FTP_NETLAB2)

.PHONY: run_netlab3
run_netlab:
	make clean && make && $(BIN)/main $(FTP_NETLAB3)

# make run arg=ftp://ftp.fe.up.pt
.PHONY: run
run:
	make clean && make && $(BIN)/main $(arg)