######################### Project Information #################################
#Name of target application
TARGET = odroid_truck
#Directory for output binary
OUTPUT_DIR = bin
#Final Binary File
BINARY = $(OUTPUT_DIR)/$(TARGET)

######################### Source to Object Translation ########################
#Directory for all sourcefiles
SRC_DIR = src
#Directoryf or all object files
OBJECT_DIR = objs

#Get all source files
SRCFILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJFILES = $(patsubst $(SRC_DIR)/%.cpp, $(OBJECT_DIR)/%.o, $(SRCFILES))

######################### Function re-definitions #############################

ECHO = echo
RM = rm -rf
MKDIR = mkdir

######################### Compiler Options ####################################

CC=g++
LD=g++
CFLAGS = -std=c++11 
LIB_PATH = /usr/local/lib
LFLAGS = -L$(LIB_PATH) -lopencv_core -lopencv_imgcodecs -lopencv_videoio -lopencv_highgui -lopencv_imgproc -lopencv_calib3d

######################### Dependencies List ###################################
.PHONY: all clean setup

all: $(BINARY)

$(BINARY): setup $(OBJECT_DIR)/$(OBJFILES)
	@$(ECHO) -n "Linking $@..."
	@$(LD) $(OBJFILES) $(LFLAGS) -o $(BINARY) 
	@$(ECHO) "Complete!"
	@$(ECHO) "Output file: $(BINARY)"

$(OBJECT_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(ECHO) -n "Compiling $<..."
	@$(CC) $(CFLAGS) -I$(SRC_DIR) -L$(LFLAGS) $(LIBS) -c $< -o $@ 
	@$(ECHO) "Done."

$(OBJECT_DIR): setup

setup:
	@$(MKDIR) -p $(OBJECT_DIR)

clean:
	@$(RM) $(BINARY) $(OBJECT_DIR)
	@$(ECHO) "Project $(TARGET) cleaned."




#CLASSES = video_capture.cpp
#
#all: $(TARGET).cpp
#	$(CXX) $(CLASSES) $(TARGET).cpp $(CFLAGS) -o $(TARGET) -L$(LFLAGS) $(LIBS)
#
#clean:
#	rm -rf $(TARGET)
