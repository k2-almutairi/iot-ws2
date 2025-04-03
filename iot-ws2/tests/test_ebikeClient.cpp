 /*this program using catch2 framework to verify that:
  - proper reading and formatting of GPS data via GPSSensor.
    -behavior of CSVHALManager when interacting with a GPS sensor.
    *in this test we assume that already we have a csv file of name "tests/test-gps.csv" which contains a GPS data and the tests are done by comparing to it*/
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../src/GPSSensor.h"
#include "../src/hal/CSVHALManager.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>


 /* helper function to read and format a GPS reading from a certain port*/
std::string printReading(GPSSensor& gps, CSVHALManager& hal, int portId) {
    auto bytes = hal.read(portId);
    return gps.format(bytes);
}


 // test case to verify thatGPS sensor makes a proper formated reading
TEST_CASE("ebikeClient prints formatted reading from GPS via HAL", "[ebikeClient]") {
    CSVHALManager hal(1);
    hal.initialise("tests/test-gps.csv");

    // declare a gps sensor using hal first constructor with id=0
    std::shared_ptr<GPSSensor> gps = std::make_shared<GPSSensor>(0);
    hal.attachDevice(0, gps);

    //read and format the data
    auto result = gps->format(hal.read(0));
    // check for the expected latitude and longitude
    REQUIRE(result.find("51.45") != std::string::npos);
    REQUIRE(result.find("-2.57") != std::string::npos);

    hal.releaseDevice(0);
}


 // verify that reading more than the expected time (more than 2) throws an exception
TEST_CASE("ebikeClient handles empty GPS data (via HAL)", "[ebikeClient]") {
    CSVHALManager hal(1);
    hal.initialise("tests/test-gps.csv");

    //declare a gps sensor using hal first constructor with id=0
    std::shared_ptr<GPSSensor> gps = std::make_shared<GPSSensor>(0);
    hal.attachDevice(0, gps);

    // Read 2 times
    hal.read(0);
    hal.read(0);

    // Third read(should throw exception
    bool caught = false;
    try {
        hal.read(0);
    } catch (const std::out_of_range&) {
        caught = true;
    }

    REQUIRE(caught == true);

    hal.releaseDevice(0);
}

