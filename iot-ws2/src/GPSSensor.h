#ifndef GPSSENSOR_H
#define GPSSENSOR_H
#include <cstdint>     // For fixed-width integer types like uint8_t
#include "hal/ISensor.h" // Interface for the sensors
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <utility>
#include <stdexcept>

 /*GPS sensor which reads latitdue ang longitude
 it implement the Isensor inteface from HAL files*/
class GPSSensor : public ISensor 
{
private:
    int id; //Sensor id
    int dimension;           // dimension(usually 2)
    std::ifstream file;   // Input file stream 

public:
  // this constructor initialize the sensor with given ID and set dimension to 2
    GPSSensor(int sensorId) : id(sensorId), dimension(2) {}

    //this constructor open provided csv file for testing in stanalone
    GPSSensor(const std::string& filepath) : id(1), dimension(2) {
        file.open(filepath);}

 //getter function for the sensor ID
    int getId() const override {
        return id;
    }

   // getter for the dimension
    int getDimension() const override {
        return dimension;}

/*
      reads a byte vector thatcontains raw reading from sensor.
      returns a string in format"latitude; longitude" ,or,a raw fallback.
     */
    std::string format(std::vector<uint8_t> reading) override {
        if (reading.empty()) return "";

        std::string rawLine(reading.begin(), reading.end());

        // Remove whitespace 
        rawLine.erase(std::remove_if(rawLine.begin(), rawLine.end(),
                        [](unsigned char c) { return std::isspace(c); }), rawLine.end());

        std::string lat, lon;
        size_t splitAt = std::string::npos;

        if(!rawLine.empty()) {
            if (rawLine[0] == '-') {
                splitAt = rawLine.find('-', 1); // Skip first '-' in latitude
            } else {
                splitAt = rawLine.find('-', 1);
            }
        }

        // Extract latitude and longitude
        if (splitAt!= std::string::npos)
        {
            lat = rawLine.substr(0, splitAt);
            lon = rawLine.substr(splitAt);
            return lat + "; " + lon;
        }

        return rawLine; //fallback if no valid split availble}

      /*reads one line from file,expects it to contain comma-separation,
     returns the formatted string in theform:"latitude; longitude".*/
    std::string readValue() {
        std::string line;
        if(std::getline(file, line))
        {
            std::istringstream ss(line);
            std::string lat, lon;
            if (!std::getline(ss, lat, ',')) return "";
            if (!std::getline(ss, lon, ',')) return "";
            return lat + "; " + lon;
        }
        return "";}

     /*
      static helper func that parse csv data line into pair of lat,lon
     */
    static std::pair<double, double> parseGPSData(const std::string& csvLine)
    {
        std::istringstream ss(csvLine);
        std::string latStr, lonStr, timestamp;

        if (!std::getline(ss, latStr, ',')) {
            throw std::runtime_error("Failed to parse latitude");
        }
        if (!std::getline(ss, lonStr, ',')) {
            throw std::runtime_error("Failed to parse longitude");
        }

        //optional timestamp reading(which not used)
        std::getline(ss, timestamp, ',');

        double lat = std::stod(latStr);
        double lon = std::stod(lonStr);
        return std::make_pair(lat, lon);}
};

#endif

