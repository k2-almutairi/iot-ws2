#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctime>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>

using namespace std;
/*
 The main function, ebike is simulated ,which send GPS data to a gateway using UDP, firstly it sends a JOIN message , then repeatidly send data (retrieved from a csv file).
 command line: ./ebikeClient <server IP> <starting ebike_id> <csv_file> <interval>
 */
int main(int argc,char* argv[]) {
    //validate the arguments of the command line
    if(argc < 5) {
        cerr << "Usage: " <<argv[0]<< " <server IP> <starting ebike_id> <csv_file> <interval>"<< endl;
        return 1;
    }

    //extract the arguments
    string serverIP =argv[1]; //IP address
    int starting_id= atoi(argv[2]);       // starting ebike ID
    string csvFile = argv[3];              // Path of the csv file
    int interval =atoi(argv[4]);          // Interval(seconds) between messages

    //simulating the HAL device attach
    cout << "[CSVHALManager] Device attached to port 0." << endl;

    //creating a UDP socket for communication
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Set the destination server(gateway) address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);  //gateway on port 8080
    inet_pton(AF_INET,serverIP.c_str(), &serverAddr.sin_addr);

    //socket timeout for recieving (2 sec)
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    //STEP 1: Send the JOIN message
    Poco::JSON::Object joinObj;
    joinObj.set("directive", "JOIN");
    joinObj.set("ebike_id",starting_id);

    time_t now = time(nullptr);
    char isoTime[30];
    strftime(isoTime, sizeof(isoTime), "%Y-%m-%d %H:%M:%S ", localtime(&now));
    joinObj.set("timestamp", string(isoTime));

    stringstream joinStr;
    Poco::JSON::Stringifier::stringify(joinObj, joinStr);
    string joinMsg = joinStr.str();

    // now sending the JOIN message to the gateway
    sendto(sock, joinMsg.c_str(),joinMsg.size(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    //wait for JACK response(no display needed)
    char ackBuffer[1024];
    struct sockaddr_in fromAddr;
    socklen_t fromLen = sizeof(fromAddr);
    recvfrom(sock, ackBuffer, sizeof(ackBuffer) - 1, 0, (struct sockaddr*)&fromAddr, &fromLen);

    // STEP 2: sending DATA messages 
    ifstream file(csvFile.c_str());
    if(!file) {
        cerr << "Error opening CSV file: " << csvFile << endl;
        return 1;
    }

    string line;
    int unique_id= starting_id;

    // loop through rows of the csv file
    while(getline(file, line)) {
        stringstream ss(line);
        string dummy, latStr, lonStr, timestamp;

        // parse the latitude,longitude, and timestamp.
        getline(ss, latStr, ',');
        getline(ss, lonStr,',');
        if(!getline(ss ,timestamp,',')) {
            time_t now= time(nullptr);
            char iso[30];
            strftime(iso,sizeof(iso), "%Y-%m-%d %H:%M:%S", localtime(&now));
            timestamp =string(iso);
        }

        // log the reading with current time
        time_t now = time(nullptr);
        char timeBuffer[30];
        strftime(timeBuffer,sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
        cout << "["<< timeBuffer << "] GPS: " <<latStr << ", " << lonStr << endl;

        // building the JSON DATA message
        Poco::JSON::Object dataObj;
        dataObj.set("directive", "DATA");
        dataObj.set("ebike_id", unique_id);
        dataObj.set("timestamp", timestamp);

        //add aGPS object to the message
        Poco::JSON::Object gpsObj;
        gpsObj.set("lat", atof(latStr.c_str()));
        gpsObj.set("lon", atof(lonStr.c_str()));
        dataObj.set("gps", gpsObj);

        // Lock status(default = 0 (unlocked))
        dataObj.set("lock_status", 0);

        //convert JSON object into string
        stringstream dataStr;
        Poco::JSON::Stringifier::stringify(dataObj, dataStr);
        string dataMessage = dataStr.str();

        //send the DATA message to UDP
        sendto(sock, dataMessage.c_str(), dataMessage.size(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

        // receive ACK for DATA
        int n = recvfrom(sock, ackBuffer, sizeof(ackBuffer) - 1, 0, (struct sockaddr*)&fromAddr, &fromLen);
        if (n > 0) {
            ackBuffer[n] = '\0';
            cout << "Received response: " << ackBuffer << endl;
            cout << "From: " << inet_ntoa(fromAddr.sin_addr) << ":" << ntohs(fromAddr.sin_port) << endl;
        }

        unique_id++;         
        sleep(interval); // wait before iterating
    }

    close(sock); // Cleanup and close the socket
    
    return 0;
}

