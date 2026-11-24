# ============================================================
#  GEC6818 智能车载中控 - Makefile
#
#  目录结构参考参考工程 project/：inc/ + src/ + res/
#  所有 src/*.c 直接编译成一个 carmode
#
#  make                 编译
#  make STATIC=1        静态链接
#  make clean           清理
#  make pack            打包 carmode + res
#  make deploy          打印 tftp 下载命令
# ============================================================

CROSS_COMPILE ?= arm-linux-
CC            := $(CROSS_COMPILE)gcc

BUILD_DIR := out
SRC_DIR   := src
INC_DIR   := inc
LOG_DIR   := log
TARGET    := carmode

CFLAGS   ?= -Wall -Wextra -O2 -std=gnu99
CPPFLAGS += -I$(INC_DIR) -I$(LOG_DIR)
LDLIBS   += -lm -lpthread

STATIC ?= 0
ifeq ($(STATIC),1)
LDFLAGS += -static
endif

TFTP_SERVER ?= 192.168.1.3
PACK_TMP    := /tmp/carmode_pkg

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/obj/%.o,$(SRCS))
LOG_OBJ := $(BUILD_DIR)/obj/log.o
BIN  := $(BUILD_DIR)/$(TARGET)

.PHONY: all clean distclean pack deploy help

all: $(BIN)

$(BIN): $(OBJS) $(LOG_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
	@echo "[OK] $(BIN)"

$(BUILD_DIR)/obj/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(LOG_OBJ): $(LOG_DIR)/log.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

# 第三方字库引擎关闭告警
$(BUILD_DIR)/obj/truetype.o: CFLAGS += -w

-include $(OBJS:.o=.d) $(LOG_OBJ:.o=.d)

clean:
	rm -rf $(BUILD_DIR)

distclean: clean

# 在 /mnt/hgfs 共享目录下，tar 直接读 res 容易报
# "file changed as we read it"，所以先暂存到 /tmp 再打包。
pack: $(BIN)
	rm -rf $(PACK_TMP)
	mkdir -p $(PACK_TMP)
	cp $(BIN) $(PACK_TMP)/carmode
	cp -r res $(PACK_TMP)/res
	tar czf $(BUILD_DIR)/carmode.tar.gz -C $(PACK_TMP) .
	rm -rf $(PACK_TMP)
	@echo "[OK] $(BUILD_DIR)/carmode.tar.gz"

deploy: $(BIN)
	@echo "在开发板上执行："
	@echo "  tftp -g -r carmode $(TFTP_SERVER)"
	@echo "  chmod +x carmode"
	@echo "  ./carmode"

help:
	@echo "make                  编译 carmode"
	@echo "make STATIC=1         静态链接"
	@echo "make clean            清理"
	@echo "make pack             打包 carmode 和 res"
	@echo "make CROSS_COMPILE=arm-linux-gnueabi-  指定工具链前缀"
