TARGET=tic

ifeq (,$(TOOLCHAIN_PREFIX))
$(error TOOLCHAIN_PREFIX is not set)
endif

ifeq (,$(CFLAGS))
$(error CFLAGS is not set)
endif

ifeq (,$(LDFLAGS))
$(error LDFLAGS is not set)
endif

CC = $(TOOLCHAIN_PREFIX)gcc

OUTPUT_DIR = out
SOURCE_DIR = src
INCLUDE_DIR = include

CFLAGS += -I$(SYSROOT)/usr/include -I$(INCLUDE_DIR)

LDFLAGS += -L$(SYSROOT)/lib
LDFLAGS += -L$(SYSROOT)/usr/lib

SOURCE = $(wildcard $(SOURCE_DIR)/*.c)
OBJS = $(patsubst $(SOURCE_DIR)/%.c,$(OUTPUT_DIR)/%.o,$(SOURCE))

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(OUTPUT_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	@rm $(TARGET) -f
	@rm $(OUTPUT_DIR) -rf

