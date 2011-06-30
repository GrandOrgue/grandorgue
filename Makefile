#
# File:     Makefile for MyOrgan
# Authors:  Johan Blok
#           Nicholas Appleton (2010-2011)
# Created:  2006

# This will be the output binary filename
TARGET                      = grandorgue

# to build in debug mode specify DEBUG=1 on the command line
# to build in unicode mode specify UNICODE=1 on the command line

# the following "DEBUG_xxx" settings will only be in effect for a debug build
# configuration.

# do you want extended release alignment debugging information dumped to the
# console?
DEBUG_RELEASE_ALIGNMENT     = yes

# when compiling a debug version of GrandOrgue, do you also want to use
# a debug version of wxWidgets?
DEBUG_WX                    = yes

# RtAudio configuration
RTAUDIO_USING_JACK          = yes
RTAUDIO_USING_ALSA          = yes
RTAUDIO_USING_OSS           = no
RTAUDIO_USING_DIRECTSOUND   = yes
RTAUDIO_USING_MM            = yes
RTAUDIO_USING_ASIO          = no

# RtMidi configuration
RTMIDI_USE_JACK             = yes
RTMIDI_USE_ALSA             = yes
RTMIDI_USE_MM               = yes

# objects to use in the build
OBJECTS =

# phony targets
.PHONY: all help clean

# detect operating system
UNAME := $(shell uname)
ifeq ($(UNAME),Linux)
SYSTEM = linux
else
SYSTEM = windows
endif

# get a build id string that will be used to determine where build files
# should be placed
ifeq ($(UNICODE),1)
ifeq ($(DEBUG),1)
GO_BUILD_ID := unicode_debug
else
GO_BUILD_ID := unicode_release
endif
else
ifeq ($(DEBUG),1)
GO_BUILD_ID += ansi_debug
else
GO_BUILD_ID += ansi_release
endif
endif

# directory to place object files
OBJ_DIR          = obj/$(SYSTEM)/$(GO_BUILD_ID)

# directory to place executable files
BIN_DIR          = bin/$(SYSTEM)/$(GO_BUILD_ID)

# directory which contains source files
SRC_DIR          = src

# include required sources in the build
include $(SRC_DIR)/grandorgue/Makefile.include
include $(SRC_DIR)/rtaudio/Makefile.include
include $(SRC_DIR)/rtmidi/Makefile.include

# object filenames for different builds
OBJ_FILES        = $(addprefix $(OBJ_DIR)/,$(OBJECTS))

# the C++ compiler
CXX             := g++

# figure out the file extension for the target
ifeq ($(SYSTEM),linux)
BIN_EXTENSION   :=
else
BIN_EXTENSION   := .exe
endif

# executable filename
TARGET_EXE       = $(BIN_DIR)/$(SYSTEM)/$(TARGET)$(BIN_EXTENSION)

# common compilation flags applying to all build modes
CXXFLAGS         = -fomit-frame-pointer -ffast-math -Wall

# get debug/release specific compile flags
ifeq ($(DEBUG),1)
CXXFLAGS        += -g -O0 -mmmx -msse -msse2 -msse3 -funit-at-a-time
else
CXXFLAGS        += -O2 -mmmx -msse -msse2 -msse3 -DNDEBUG
endif

# phony targets
help:
	@echo "GrandOrgue Makefile"
	@echo "Type:"
	@echo "  make help          To display this message."
	@echo "  make all           Will build all currently supported configurations"
	@echo "                     of GrandOrgue."
	@echo "  make clean         Will erase all compiled object files and all built"
	@echo "                     executables for this system."

all: $(TARGET_EXE) # the unicode targets should be added once supported

clean: 
	rm -f $(TARGET_EXE) $(OBJ_FILES)
	
# executable targets
$(TARGET_EXE): $(OBJ_FILES)
	@test -d $(dir $@) || mkdir -p $(dir $@)
	$(CXX) -o $(TARGET_EXE) $(OBJ_FILES) $(LDFLAGS)
