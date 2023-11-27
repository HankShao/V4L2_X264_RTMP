PROJECT = $(shell pwd)
SRC_PATH := $(PROJECT)
SRC_PATH += $(PROJECT)/module

#查找所有路径下的源文件
OBJ_C := $(foreach dir, $(SRC_PATH), $(wildcard $(dir)/*.c))
#得到目标文件列表
OBJ := $(patsubst %.c, %.o, $(OBJ_C))

OBJ_CPP :=  $(foreach dir, $(SRC_PATH), $(wildcard $(dir)/*.cpp))
OBJ_O_CPP := $(patsubst %.cpp, %.o, $(OBJ_CPP))

CFLAGS := -g -O0
CFLAGS += -Wall

CFLAGS += -I$(PROJECT)/include
CFLAGS += -L$(PROJECT)/lib
CFLAGS += -lpthread -lm
CFLAGS += -lx264 -ldl

TARGET = sample

all:$(OBJ) $(OBJ_O_CPP)
#	@echo $(OBJ_C)
#	#@echo $(OBJ)
	g++ $^ -o $(TARGET) $(CFLAGS)

%.o:%.c
	gcc $(CFLAGS) -c $^ -o $@
%.o:%.cpp
	g++ $(CFLAGS) -std=c++11 -c $^ -o $@

clean:
	@rm -f $(TARGET) $(OBJ) $(OBJ_O_CPP)
