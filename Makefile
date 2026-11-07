BIN = ./bin/game
INCLUDES = -I./src -I./src/libsys

CFLAGS = -std=c99 -Wall -Wno-missing-braces -g -DDEBUG -DMEM_DEBUG -O3

BASE_SRCS = src/libsys/*.c \
			src/libsys/audio/*.c \
			src/libsys/graphics/*.c \
			src/*.c \
			src/levels/*.c \
			src/menu/*.c

# PLATFORM defaults to the host; `make PLATFORM=windows` cross-compiles with mingw.
UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
	PLATFORM ?= macos
else ifeq ($(UNAME),Linux)
	PLATFORM ?= linux
else
	PLATFORM ?= windows
endif

# AVX2 only exists on x86-64 (Apple Silicon and other arm64 hosts skip it).
ifeq ($(shell uname -m),x86_64)
	ARCH_FLAGS = -mavx2
endif

TARGET = dke

ifeq ($(PLATFORM),macos)
	CC ?= clang
	CFLAGS += -DPLATFORM_MACOS $(ARCH_FLAGS)
	LDFLAGS = -framework Cocoa -framework AudioToolbox
	SRCS = src/libsys/platforms/macos.m $(BASE_SRCS)
endif
ifeq ($(PLATFORM),linux)
	CC = clang
	CFLAGS += -DPLATFORM_LINUX -D_DEFAULT_SOURCE $(ARCH_FLAGS)
	LDFLAGS = -lX11 -lpthread -lm -ldl
	SRCS = src/libsys/platforms/linux.c $(BASE_SRCS)
endif
ifeq ($(PLATFORM),windows)
	CC = x86_64-w64-mingw32-gcc
	CFLAGS += -DPLATFORM_WINDOWS -mavx2
	LDFLAGS = -lmingw32 -mwindows -lgdi32 -lwinmm -static -lpthread
	SRCS = src/libsys/platforms/windows.c $(BASE_SRCS)
	TARGET = dke.exe
endif

dke:
	$(CC) $(INCLUDES) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $(BIN)/$(TARGET)

run:
	cd $(BIN) && ./dke

clean:
	rm -f $(BIN)/dke $(BIN)/dke.exe

.PHONY: dke run clean

# PlayStation Vita: platforms/vita/CMakeLists.txt -> build/vita/dke.vpk
VITASDK ?= $(HOME)/vitasdk
.PHONY: vita
vita:
	VITASDK=$(VITASDK) PATH="$(VITASDK)/bin:$$PATH" cmake -S platforms/vita -B build/vita
	VITASDK=$(VITASDK) PATH="$(VITASDK)/bin:$$PATH" cmake --build build/vita --parallel
