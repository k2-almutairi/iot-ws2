#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <string>
#include <vector>
#include <mutex>
#include <iostream>
#include <sstream>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/Dynamic/Var.h>

/*
 Structure  represent an eBike GeoJSON feature data
 */
struct GeoJSONFeature {
    std::string type;  // Always "Feature as per GeoJSON standard
    double longitude; // longitude
    double latitude;// GPS latitude
    int ebike_id; // Unique ID 
    std::string timestamp; // timestamp 
    int lock_status;  // 0 = unlocked, 1 = locked
};

class MessageHandler
{
public:
    static std::vector<GeoJSONFeature> features;  //container of features
    static std::mutex featuresMutex;

    /*
     handle the recieved JSON messages using "DATA" directive and parse the gps data to store it in the feature list
     */
    static void handleDataMessage(Poco::JSON::Object::Ptr obj)
    {
        int ebike_id = obj->getValue<int>("ebike_id");
        std::string timestamp = obj->getValue<std::string>("timestamp");
        Poco::JSON::Object::Ptr gpsObj = obj->getObject("gps");
        double lat =gpsObj->getValue<double>("lat");
        double lon= gpsObj->getValue<double>("lon");
        int lock_status =obj->optValue("lock_status", 0); // ebike unlocked by dafaultt

        GeoJSONFeature feature;
        feature.type = "Feature";
        feature.ebike_id = ebike_id;
        feature.timestamp= timestamp;
        feature.latitude =lat;
        
        
        
        feature.longitude = lon;
        feature.lock_status= lock_status;

        std::lock_guard<std::mutex> lock(featuresMutex);
        features.push_back(feature);
    }

    /*
     "COMMAND" directive, updates the status of a certain ebike(locked or unlocked)
     */
    static std::string handleCommandMessage(Poco::JSON::Object::Ptr obj)
    {
        std::string action = obj->getValue<std::string>("action");//"lock" or "unlock"
        Poco::JSON::Array::Ptr ids = obj->getArray("ebike_ids");

        std::lock_guard<std::mutex> lock(featuresMutex);
        for (size_t i = 0; i < ids->size(); ++i) {
            int target_id = ids->getElement<int>(i);
            for (auto& feature : features) {
                if (feature.ebike_id ==target_id) {
                    feature.lock_status= (action == "lock") ? 1 : 0;
                }
            }
        }

        //prepare the COMMACK respons
        Poco::JSON::Object::Ptr ack =new Poco::JSON::Object();
        ack->set("directive", "COMMACK");
        ack->set("status", "OK");
        std::ostringstream oss;
        Poco::JSON::Stringifier::stringify(ack, oss);
        return oss.str();
    }

    /*
     handling the JOIN, DATA, COMMAND directories
     */
    static void handleMessage(const std::string& message)
    {
        try {
            Poco::JSON::Parser parser;
            Poco::Dynamic::Var result= parser.parse(message);
            Poco::JSON::Object::Ptr obj = result.extract<Poco::JSON::Object::Ptr>();
            std::string directive = obj->getValue<std::string>("directive");

            if (directive =="JOIN") {
                //handle registration or logging if needed
            } else if (directive=="DATA") {
                handleDataMessage(obj);
            } else if (directive== "COMMAND") {
                std::string action = obj->getValue<std::string>("action");
                int new_status= (action =="lock") ? 1 : 0;

                Poco::JSON::Array::Ptr idArray = obj->getArray("ebike_ids");

                std::lock_guard<std::mutex> lock(featuresMutex);
                for (size_t i = 0; i < idArray->size(); ++i){
                    int targetId = idArray->getElement<int>(i);
                    for (auto it = features.rbegin(); it != features.rend(); ++it) {
                        if (it->ebike_id ==targetId) {
                            it->lock_status= new_status;
                            break;
                        }
                    }
                }
            }
        } catch (std::exception &ex) {
            std::cerr << "Error processing message: " << ex.what() << std::endl;
        }
    }

    /*
     extrating the directive field from JSON message
     and return the type of directory
     */
    static std::string extractDirective(const std::string& message) {
        try{
            Poco::JSON::Parser parser;
            Poco::Dynamic::Var result = parser.parse(message);
            Poco::JSON::Object::Ptr obj = result.extract<Poco::JSON::Object::Ptr>();
            return obj->getValue<std::string>("directive");
        } catch (...){
            return "";
        }
    }

    /*
     construct ans return COMMACK MESSSAGE and return a commack string
     */
    static std::string buildCOMMACK() {
        Poco::JSON::Object::Ptr ack = new Poco::JSON::Object();
        ack->set("directive", "COMMACK");
        std::ostringstream oss;
        Poco::JSON::Stringifier::stringify(ack, oss);
        return oss.str();
    }
};

/*static members*/
std::vector<GeoJSONFeature> MessageHandler::features;
std::mutex MessageHandler::featuresMutex;

#endif

