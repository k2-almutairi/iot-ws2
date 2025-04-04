# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

# Paths
SRC_DIR = src
BUILD_DIR = build
LIB_DIR = /opt/iot/lib
POCO_DIR = /opt/iot/poco

# Include and library paths for POCO
INCLUDES = -I$(POCO_DIR)/include -I$(SRC_DIR) -I$(SRC_DIR)/web
LIBS = -L$(LIB_DIR) -lwebserver \
       -L$(POCO_DIR)/lib -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON -lpthread
LDFLAGS = -Wl,-rpath,$(POCO_DIR)/lib

# Target executables
CLIENT_TARGET = ebikeClient
SERVER_TARGET = ebikeGateway
GEN_TARGET = generateEBikeFile
MGMT_TARGET = managementClient

# Source files
CLIENT_SRC = $(SRC_DIR)/ebikeClient.cpp
SERVER_SRC = $(SRC_DIR)/ebikeGateway.cpp
GEN_SRC = $(SRC_DIR)/util/generateEBikeFile.cpp
MGMT_SRC = $(SRC_DIR)/managementClient.cpp

# Object files
CLIENT_OBJ = $(BUILD_DIR)/ebikeClient.o
SERVER_OBJ = $(BUILD_DIR)/ebikeGateway.o
GEN_OBJ = $(BUILD_DIR)/util/generateEBikeFile.o
MGMT_OBJ = $(BUILD_DIR)/managementClient.o

# Build rules
all: $(BUILD_DIR) $(CLIENT_TARGET) $(SERVER_TARGET) $(GEN_TARGET) $(MGMT_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/web
	mkdir -p $(BUILD_DIR)/util

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS) $(LDFLAGS)

$(SERVER_TARGET): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS) $(LDFLAGS)

$(GEN_TARGET): $(GEN_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

$(MGMT_TARGET): $(MGMT_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/util/%.o: $(SRC_DIR)/util/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean up
clean:
	rm -rf $(BUILD_DIR) $(CLIENT_TARGET) $(SERVER_TARGET) $(GEN_TARGET) $(MGMT_TARGET)

.PHONY: all clean

