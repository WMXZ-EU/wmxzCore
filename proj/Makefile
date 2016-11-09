#TARGET = doDecode
#SRC_LIST = doDecode.c

TARGET = testCompress

SRC_DIR = ..\examples\$(TARGET)
SRC_LIST = $(TARGET).c

X3_DIR = ..\src
X3_LIST =  compress.c

BUILD_DIR = build
BIN_DIR = bin


OS := WINDOWS
CC = gcc

CFLAGS := -O2 -Wall

SRC_C = $(wildcard $(addprefix $(SRC_DIR)/, $(SRC_LIST))) 
X3_C = $(wildcard $(addprefix $(X3_DIR)/, $(X3_LIST))) 

C_SOURCES := $(SRC_C) $(X3_C)

OBJS := $(C_SOURCES:.c=.o) 
BUILDS := $(addprefix $(BUILD_DIR)\, $(notdir $(OBJS)))

#search path for pre-requisites (seperate by ';')
VPATH = $(SRC_DIR);$(X3_DIR)

#generate path list
PATH_LIST = $(subst ;, ,$(VPATH))

INCLUDES := $(patsubst %,-I%,$(PATH_LIST)) 
LIBS = -lm

#-------------------------------------------------------------
default:
	@dir $(X3_DIR)
	@echo $(X3_C)
	@echo $(C_SOURCES)
	@echo $(INCLUDES)

all: $(TARGET).exe
	
$(TARGET).exe: $(BUILDS)
	@echo "[LD]	$@"
	$(CC) -o $(BIN_DIR)\$@ $(BUILDS) -L$(LIB_DIR) $(LIBS)

$(BUILD_DIR)\\%.o: %.c .\Makefile
	@echo "[CC]	$<"
	$(CC)  $(CFLAGS) $(INCLUDES) -o "$@" -c "$<"

# compiler generated dependency info
-include $(BUILDS:.o=.d)
include

clean:
	@echo Cleaning...
	@del $(BUILD_DIR)\*.o $(BUILD_DIR)\*.d $(BUILD_DIR)\*.lis
