CXX = g++
POCO_DIR = $(HOME)/poco-local
CXXFLAGS = -std=c++11 -Wall -I$(POCO_DIR)/include
LDFLAGS = -L$(POCO_DIR)/lib -lPocoJSON -lPocoFoundation -lPocoNet

CLIENT_SRC = src/ebikeClient.cpp
GATEWAY_SRC = src/ebikeGateway.cpp
GEN_SRC = src/util/generateEBikeFile.cpp
MGMT_SRC = src/managementClient.cpp

BUILD_DIR = build

all: ebikeClient ebikeGateway generateEBikeFile managementClient

ebikeClient: $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(CLIENT_SRC) $(LDFLAGS)
	chmod +x $@

ebikeGateway: $(GATEWAY_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(GATEWAY_SRC) $(LDFLAGS)
	chmod +x $@

generateEBikeFile: $(GEN_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(GEN_SRC)
	chmod +x $@

managementClient: $(MGMT_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(MGMT_SRC) $(LDFLAGS)
	chmod +x $@

clean:
	rm -f ebikeClient ebikeGateway generateEBikeFile managementClient

.PHONY: all clean

