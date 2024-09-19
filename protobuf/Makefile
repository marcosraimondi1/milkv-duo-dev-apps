TARGET=proto_example

# ifeq (,$(TOOLCHAIN_PREFIX))
# $(error TOOLCHAIN_PREFIX is not set)
# endif
#
# ifeq (,$(CFLAGS))
# $(error CFLAGS is not set)
# endif
#
# ifeq (,$(LDFLAGS))
# $(error LDFLAGS is not set)
# endif

INCLUDE_DIR = include
SOURCE_DIR = src
OUTPUT_DIR = out

CC = $(TOOLCHAIN_PREFIX)gcc

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
	@rm -rf $(OUTPUT_DIR)
	@rm $(TARGET)

