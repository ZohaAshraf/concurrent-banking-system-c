CC      = gcc
CFLAGS  = -Wall -Wextra -g -pthread
LDFLAGS = -pthread -lrt

SRC_DIR = src
SRCS    = $(SRC_DIR)/main.c \
          $(SRC_DIR)/accounts.c \
          $(SRC_DIR)/scheduling.c \
          $(SRC_DIR)/sync.c \
          $(SRC_DIR)/banker.c \
          $(SRC_DIR)/ipc.c \
          $(SRC_DIR)/memory.c \
          $(SRC_DIR)/utils.c

TARGET  = banking_system

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p logs charts
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build successful: ./$(TARGET)"

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET) logs/*.log
