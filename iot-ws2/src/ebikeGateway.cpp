#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <fstream>
#include <vector>
#include <mutex>

// POCO networking and JSON headers
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>
#include "SocketServer.h"
#include "MessageHandler.h"
#include "web/EbikeHandler.h"//interface for web handler

using namespace Poco::Net;
using namespace Poco::JSON;
using namespace Poco;
using namespace std;

/*
 for Handling HTTp requests to serve the map and JSON data
 */
class MapRequestHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        string uri = request.getURI();

        
        //for serving the html page(html.map)
        if (uri == "/"|| uri =="/map.html") {
            ifstream file("src/html/map.html");
            if (file) {
                response.setStatus(HTTPResponse::HTTP_OK);
                response.setContentType("text/html");
                ostringstream oss;
                oss << file.rdbuf(); // Read from the file
                string html = oss.str();
                response.sendBuffer(html.data(), html.size());
            }else{
                response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                response.send() << "map.html not found";
            }}

        // serves real-time data for map and table(in html.map)
        else if(uri== "/ebikes")
        {
            response.setStatus(HTTPResponse::HTTP_OK);
            response.setContentType("application/json");

            //create the root GeoJSON feature colection
            Object::Ptr featureCollection = new Object();
            featureCollection->set("type", "FeatureCollection");

            // Array for individual bikes
            Array::Ptr featuresArray= new Array();
            {
                lock_guard<mutex> lock(MessageHandler::featuresMutex);
                for(auto& feature : MessageHandler::features)
                {
                    Object::Ptr featureObj =new Object();
                    featureObj->set("type", "Feature");

                    //lat/lon coordinates
                    Object::Ptr geometry = new Object();
                    geometry->set("type", "Point");
                    Array::Ptr coordinates= new Array();
                    coordinates->add(feature.longitude);  // GeoJSON uses [lon, lat]
                    coordinates->add(feature.latitude);
                    geometry->set("coordinates", coordinates);
                    featureObj->set("geometry", geometry);

                    // Properties block:metadata for eBike(in the table below the map)
                    Object::Ptr properties =new Object();
                    properties->set("id", feature.ebike_id);
                    properties->set("timestamp", feature.timestamp);
                    properties->set("status", feature.lock_status == 1 ? "locked" : "unlocked");
                    featureObj->set("properties", properties);

                    featuresArray->add(featureObj);
                }
            }

            // attach an array to colection and respond along with JSON
            featureCollection->set("features", featuresArray);
            ostringstream oss;
            Stringifier::stringify(featureCollection, oss);
            string jsonStr = oss.str();
            response.sendBuffer(jsonStr.data(), jsonStr.size());}

        // Handle unkown roots or exceptions
        else
        {
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            response.send()<< "Not Found";
        }}
};

/*
 which returns handler for incoming HTTP requests
 */
RequestHandlerFactory::RequestHandlerFactory(Poco::JSON::Array::Ptr& ebikes)
    : _ebikes(ebikes)
{
    std::cout <<"RequestHandlerFactory constructed." << std::endl;
}

HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request) {
    return new MapRequestHandler();}

/*
 main function contain a UDP than listens for ebike messages then a webserver that serves HTML and JSON data
 */
int main()
{
    /*shared ebike JSON array */
    Poco::JSON::Array::Ptr ebikeArray = new Poco::JSON::Array();

    // Configur the IP and port to bind HTTP server
    unsigned short httpPort = 9761;
    std::string specificIP = "127.0.0.1";  // IP used(adjustable)
    Poco::Net::SocketAddress addr(specificIP, httpPort);
    ServerSocket serverSocket(addr, 64);  // 64=backlog size

    HTTPServerParams* params = new HTTPServerParams;

    //creates the HTTP server
    RequestHandlerFactory factory(ebikeArray);
    HTTPServer httpServer(&factory, serverSocket, params);

    // starting the server thread
    thread httpThread([&]() {
        httpServer.start();
        cout<< "Server started on http://" << specificIP << ":" << httpPort << endl;
        cout << "Press Ctrl+C to stop the server..." << endl;
        while (true)
        {
            this_thread::sleep_for(chrono::seconds(10));}
    });

    //starting the socket on port 8080
    cout << "Socket Server waiting for messages..." << endl;
    SocketServer udpServer(8080);
    udpServer.start();

    // if exists
    httpThread.join();
    return 0;
}

