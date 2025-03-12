# Compiler
CXX = g++

# Include and Library Flags
PKG_CONFIG = `pkg-config --cflags --libs opencv4`
LIBS = -lportaudio -lsndfile -laubio 

# Source and Output
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

#Source Files 
SRC_FILES = $(SRC_DIR)/main.cpp $(SRC_DIR)/Record_BabySounds.cpp $(SRC_DIR)/ADProcess_BabySounds.cpp $(SRC_DIR)/EmotionRecognizer.cpp $(SRC_DIR)/FaceDetection.cpp

#Object Files
OBJ_FILES = $(BUILD_DIR)/main.o $(BUILD_DIR)/Record_BabySounds.o $(BUILD_DIR)/ADProcess_BabySounds.o $(BUILD_DIR)/EmotionRecognizer.o $(BUILD_DIR)/FaceDetection.o

# OUTPUT exe
OUTPUT = BabySounds

# Default Target
all: $(OUTPUT)

# Compilation rule for exe
$(OUTPUT):$(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $(OUTPUT) $(PKG_CONFIG) $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
		$(CXX) -I$(INCLUDE_DIR) -c $< -o $@ $(PKG_CONFIG) $(LIBS)


# Compilation rule to compile .cpp -> .o
$(OUTPUT): $(SRC_DIR)
	$(CXX) $(SRC_FILES) -o $(OUTPUT) $(PKG_CONFIG) $(LIBS)

# Ensure the build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean Rule
clean:
	rm -rf $(BUILD_DIR) $(OUTPUT)
