# Eight Wings: build, test, run.
# Built on the shared sdl-2d-game-engine (git submodule in engine/), consumed the same way
# ~/dev/games/super-fabio-bro does: the engine is built by its own makefile and used through
# its library, public header directories and link flags. The engine is never modified here.
.DEFAULT_GOAL := all

CC ?= cc
SDL2_CONFIG ?= $(shell command -v sdl2-config 2>/dev/null || echo /opt/homebrew/bin/sdl2-config)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
    INSTALL_CMD = git submodule update --init --recursive && sudo apt-get update && sudo apt-get install -y libsdl2-dev libsdl2-image-dev
else ifeq ($(UNAME_S), Darwin)
    INSTALL_CMD = git submodule update --init --recursive && brew install sdl2 sdl2_image
else
    $(error Unsupported platform)
endif

ENGINE_DIR = engine
ENGINE_LIB = $(ENGINE_DIR)/libsdl2d.a
ENGINE_MAKE = $(MAKE) --no-print-directory -C $(ENGINE_DIR) WITH_TTF=0
ENGINE_INCLUDES := $(foreach dir,$(shell $(ENGINE_MAKE) -s print-public-includes),-I$(ENGINE_DIR)/$(dir))
ENGINE_LFLAGS := $(shell $(ENGINE_MAKE) -s print-link-flags)
# The pure parts of the engine (rng, geometry, stepper, controls, mixer, events, pool) plus collision:
# compiled straight into the tests, so the rules run with no window or sound device.
ENGINE_PURE := $(addprefix $(ENGINE_DIR)/,$(shell $(ENGINE_MAKE) -s print-headless-sources) core/math/collision.c)

GAME_SRC := $(wildcard src/game/*.c)
PLAT_SRC := $(wildcard src/platform/*.c)
TEST_SRC := $(wildcard tests/test_*.c)
BUILD := build/$(if $(SAN),san,rel)
OPT ?= -O2
# No platform include path and stub headers first: any attempt to include SDL is a compile error.
CFLAGS := -std=c11 -g $(OPT) -Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -Wno-float-conversion -pedantic \
          -D_DEFAULT_SOURCE -Iscripts/boundary_stubs -Isrc $(ENGINE_INCLUDES) $(EXTRA_CFLAGS)
ifdef SAN
  CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -O1
  LDSAN := -fsanitize=address,undefined
endif
GAME_OBJ := $(patsubst %.c,$(BUILD)/%.o,$(GAME_SRC))
PLAT_OBJ := $(patsubst %.c,$(BUILD)/%.o,$(PLAT_SRC))
PURE_OBJ := $(patsubst %.c,$(BUILD)/%.o,$(ENGINE_PURE))
TARGET := eight-wings
ATLAS := assets_cooked/atlas.rgba

.PHONY: all assets audio test sanitize run clean install validate screenshots boundary help FORCE
all: $(TARGET)

$(TARGET): $(GAME_OBJ) $(PLAT_OBJ) $(ENGINE_LIB) $(ATLAS)
	$(CC) -o $@ $(GAME_OBJ) $(PLAT_OBJ) $(ENGINE_LIB) $(ENGINE_LFLAGS) $(LDSAN)

# Always asked; the engine's makefile decides whether anything is stale.
$(ENGINE_LIB): FORCE
	$(ENGINE_MAKE) libsdl2d.a
FORCE:

# The sprite atlas as raw RGBA, cooked from the generated art (stdlib Python only).
$(ATLAS): tools/pack_atlas.py $(wildcard assets/generated/*/*.png) assets/spec/sprites.spec
	python3 tools/pack_atlas.py

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<
-include $(GAME_OBJ:.o=.d) $(PLAT_OBJ:.o=.d)

# Tests link the deterministic logic and the pure engine parts only.
$(BUILD)/tests/%: tests/%.c $(GAME_OBJ) $(PURE_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Itests -o $@ $< $(GAME_OBJ) $(PURE_OBJ) -lm $(LDSAN)

TEST_BIN := $(patsubst tests/%.c,$(BUILD)/tests/%,$(TEST_SRC))

boundary:
	python3 scripts/check_boundary.py

test: boundary $(TEST_BIN)
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
	$(ENGINE_MAKE) clean
	rm -rf build $(TARGET) assets_cooked

help:
	@echo "make [run|test|sanitize|assets|audio|validate|screenshots|install|clean]"
