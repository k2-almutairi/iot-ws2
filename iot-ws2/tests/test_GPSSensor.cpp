
/*this program using catch2 framework to verify basic propertion(sensor id and Dimnsion) and also the ability to read and format gps data 
    *in this test we assume that already we have a csv file of name "tests/test-gps.csv" which contains a GPS data and the tests are done by comparing to it*/
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../src/GPSSensor.h"
#include <string>


 
 //this test verify that the sensor ID is set to 0 and  dimension is 2 after declaring it.
 
TEST_CASE("GPSSensor basic properties", "[GPSSensor]") {
    GPSSensor gps("tests/test-gps.csv");
    
    SECTION("Sensor ID is correct") {
        // expect the is to be 0
        REQUIRE(gps.getId() == 0);
    } 

    SECTION("Sensor dimension is 2") {
        // expect the dimension to be 2 or throw exception
        REQUIRE(gps.getDimension() == 2);
    }
}

 /*thi test verify that the gps sensor read correctly 2 lines from the data in the csv file  and format it correctly in form :"latitude; longitude" and ensure that the end of the string is empty reading. */
TEST_CASE("GPSSensor reads and formats GPS data", "[GPSSensor]") {
  
    GPSSensor gps("tests/test-gps.csv");
    
    SECTION("First reading is correctly formatted") {
        // call readValue() should get us "51.45; -2.57"
        std::string reading = gps.readValue();
        REQUIRE(reading == "51.45; -2.57");
    }

    SECTION("Second reading is correctly formatted") {
        // skip the first value and read the second
        gps.readValue(); // skip first
        std::string reading = gps.readValue(); // get second read
        REQUIRE(reading == "51.46; -2.58");
        // throw exception if wrong
    }

    SECTION("Reading past end of file returns empty string") {
        // read two available lines and ensure that the third is empty
        gps.readValue(); // first
        gps.readValue(); // second
        std::string reading = gps.readValue(); // third read
        REQUIRE(reading == "");
    }
}

