PROJECT = $(shell pwd)
SRC_PATH := $(PROJECT)
SRC_PATH += $(PROJECT)/module

#查找所有路径下的源文件
OBJ_C := $(foreach dir, $(SRC_PATH), $(wildcard $(dir)/*.c))
#得到目标文件列表
OBJ := $(patsubst %.c, %.o, $(OBJ_C))

CFLAGS := -g -O0
CFLAGS += -Wall

CFLAGS += -I$(PROJECT)/include
CFLAGS += -L$(PROJECT)/lib
CFLAGS += -lpthread -lm


TARGET = sample

all:$(OBJ)
#	@echo $(OBJ_C)
#	#@echo $(OBJ)
	gcc $^ -o $(TARGET) $(CFLAGS)

%.o:%.c
	gcc $(CFLAGS) -c $^ -o $@


clean:
	rm -f $(TARGET) $(OBJ)
