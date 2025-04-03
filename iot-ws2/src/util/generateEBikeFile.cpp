#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <iomanip>

// Constants
const double LAT_MIN = 51.45;
const double LAT_MAX = 51.46;
const double LON_MIN = -2.6;
const double LON_MAX = -2.5;
const std::string FILE_PREFIX = "data/sim-eBike-";
const double VARIATION = 0.0001;

// Function to generate a random coordinate within a given range
double randomCoord(double min, double max, std::mt19937& gen) {
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

// Function to slightly change the coordinates to simulate movement
void varyCoord(double& lat, double& lon, std::mt19937& gen) {
    std::uniform_real_distribution<> dis(-VARIATION, VARIATION);
    lat += dis(gen);
    lon += dis(gen);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <seed> <num_files> <num_rows>" << std::endl;
        return 1;
    }

    int seed = std::stoi(argv[1]);
    int numFiles = std::stoi(argv[2]);
    int numRows = std::stoi(argv[3]);

    std::mt19937 gen(seed);

    for (int fileIndex = 0; fileIndex < numFiles; ++fileIndex) {
        std::string fileName = FILE_PREFIX + std::to_string(fileIndex + 1) + ".csv";
        std::ofstream outFile(fileName);

        if (!outFile.is_open()) {
            std::cerr << "Failed to open file: " << fileName << std::endl;
            return 1;
        }

        double lat = randomCoord(LAT_MIN, LAT_MAX, gen);
        double lon = randomCoord(LON_MIN, LON_MAX, gen);

        for (int rowIndex = 0; rowIndex < numRows; ++rowIndex) {
            outFile << std::fixed << std::setprecision(6) << lat << "," << lon << "\n";

            varyCoord(lat, lon, gen);
        }

        outFile.close();
    }

    return 0;
}
