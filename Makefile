TARGET   = game_of_life
CC       = gcc
CFLAGS   = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=199309L -Iinclude -MMD -MP
LDFLAGS  = -lncurses

SRC_DIR  = src
OBJ_DIR  = obj

SOURCES  = $(wildcard $(SRC_DIR)/*.c)
OBJECTS  = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS     = $(OBJECTS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
