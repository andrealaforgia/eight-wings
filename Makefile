# Eight Wings — build, test, run.   (Same conventions as ~/dev/games/super-fabio-bro: plain Make, sdl2-config.)
CC ?= cc
UNAME_S := $(shell uname -s)
SDL2_CONFIG ?= $(shell command -v sdl2-config 2>/dev/null || echo /opt/homebrew/bin/sdl2-config)
SDL_CFLAGS := $(shell $(SDL2_CONFIG) --cflags)
SDL_LIBS := $(shell $(SDL2_CONFIG) --libs)
ifeq ($(UNAME_S),Darwin)
  GL_LIBS := -framework OpenGL
  INSTALL_CMD = brew install sdl2 sdl2_image
else
  GL_LIBS := -lGL
  INSTALL_CMD = sudo apt-get update && sudo apt-get install -y build-essential libsdl2-dev libsdl2-image-dev libgl1-mesa-dev
endif
# SDL2_image: found via pkg-config if available, else next to SDL2.
IMG_LIBS := -lSDL2_image
OPT ?= -O2
WARN := -Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -Wno-float-conversion -pedantic
CFLAGS := -std=c11 -g $(OPT) $(WARN) -D_DEFAULT_SOURCE -Isrc $(EXTRA_CFLAGS)

CORE_SRC := $(wildcard src/core/*.c)
GAME_SRC := $(wildcard src/game/*.c)
PLAT_SRC := $(wildcard src/platform/*.c)
TEST_SRC := $(wildcard tests/test_*.c)
BUILD := build/$(if $(SAN),san,rel)
ifdef SAN
  CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -O1
  LDSAN := -fsanitize=address,undefined
endif
LOGIC_OBJ := $(patsubst %.c,$(BUILD)/%.o,$(CORE_SRC) $(GAME_SRC))
PLAT_OBJ := $(patsubst %.c,$(BUILD)/%.o,$(PLAT_SRC))
TARGET := eight-wings

.PHONY: all assets audio test sanitize run clean install check validate screenshots help
all: $(TARGET)

$(TARGET): $(LOGIC_OBJ) $(PLAT_OBJ)
	$(CC) -o $@ $^ $(SDL_LIBS) $(IMG_LIBS) $(GL_LIBS) -lm $(LDSAN)

$(BUILD)/src/platform/%.o: src/platform/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

# Unit/scenario tests link ONLY the deterministic logic (no SDL, no GL, no window).
$(BUILD)/tests/%: tests/%.c $(LOGIC_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Itests -o $@ $< $(LOGIC_OBJ) -lm $(LDSAN)

TEST_BIN := $(patsubst tests/%.c,$(BUILD)/tests/%,$(TEST_SRC))

test: $(TEST_BIN)
	@fail=0; for t in $(TEST_BIN); do echo "== $$t"; $$t || fail=1; done; \
	python3 tools/validate_assets.py || fail=1; \
	if [ $$fail -ne 0 ]; then echo "TESTS FAILED"; exit 1; fi; echo "ALL TESTS PASSED"

sanitize:
	$(MAKE) SAN=1 test

assets:
	python3 tools/gen_assets.py
	python3 tools/pack_atlas.py

audio:
	python3 tools/audiogen/gen_audio.py

validate:
	python3 tools/validate_assets.py

run: $(TARGET)
	./$(TARGET)

screenshots: $(TARGET)
	./tools/take_screenshots.sh

install:
	$(INSTALL_CMD)

clean:
	rm -rf build $(TARGET)

help:
	@echo "make            build the game        make test       run all tests + asset validation"
	@echo "make run        play                   make sanitize   tests under ASan+UBSan"
	@echo "make assets     regenerate art+atlas   make audio      regenerate sound"
	@echo "make screenshots capture real gameplay screenshots"

-include $(shell find build -name '*.d' 2>/dev/null)
