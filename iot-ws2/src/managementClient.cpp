#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctime>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Stringifier.h>

using namespace std;

/*
 prints the usage info for management client(command line arguments)
 */
void usage(const char* progName)
{
    cout << "Usage: " << progName << " <server IP> <action> <comma-separated eBike IDs> [timestamp]" << endl;
    cout << "Example: " << progName << " 127.0.0.1 lock 1,2,3 2025-02-27T12:00:00Z" << endl;
}

int main(int argc,char* argv[])
{
    // validation of the number of arguments gotten by the command line
    if(argc< 4)
    {
        usage(argv[0]);
        return 1;
    }
    
    // initiate variable from command-line arguments
    string serverIP = argv[1];
    string action = argv[2]; // "lock" or "unlock"
    string idsStr = argv[3];
    string timestamp;

    //if  the timestamp is not provided,use the current system time 
    if(argc >=5) 
    {
        timestamp =argv[4];
    } else 
    {
        time_t now= time(nullptr);
        char isoTime[30];
        strftime(isoTime, sizeof(isoTime), "%Y-%m-%dT%H:%M:%SZ", localtime(&now));
        timestamp = isoTime;
    }
    
  
    //convert the iDs separated by comma(inputted in command line) into vector
    vector<int> ebikeIDs;
    stringstream ss(idsStr);
    string token;
    while(getline(ss, token, ',')){
        ebikeIDs.push_back(atoi(token.c_str()));
    }
    
    // creating a JSON object 
    Poco::JSON::Object cmdObj;
    cmdObj.set("directive", "COMMAND");
    cmdObj.set("action", action); //lock or unlock
    cmdObj.set("timestamp", timestamp);

    //populate eBike ID array
    Poco::JSON::Array::Ptr idArray =new Poco::JSON::Array();
    for(auto id : ebikeIDs) {
        idArray->add(id);
    }
    cmdObj.set("ebike_ids", idArray);
    
    // make JSON command into string
    stringstream jsonStr;
    Poco::JSON::Stringifier::stringify(cmdObj, jsonStr);
    string message =jsonStr.str();
    
    //make a UDP socket
    int sock= socket(AF_INET, SOCK_DGRAM, 0);
    if(sock <0) {
        perror("Socket creation failed");
        return 1;
    }
    
    //setup the server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port =htons(8080);
    inet_pton(AF_INET,serverIP.c_str(), &serverAddr.sin_addr);
    
    //send the command togateway
    ssize_t sent =sendto(sock, message.c_str(), message.size(), 0,
                          (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(sent<0) {
        perror("sendto failed");
        close(sock);
        return 1;
    }

    //client side confirmation log
    cout << "[INFO] COMMAND sent to eBike(s): " << idsStr <<" [action: " << action<< "]" << endl;

    // wait for  a COMMACK from gateway
    char buffer[1024];
    struct sockaddr_in fromAddr;
    socklen_t fromLen= sizeof(fromAddr);
    int n = recvfrom(sock, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&fromAddr, &fromLen);

    if(n>0) {
        buffer[n] = '\0';
        cout << "[INFO] COMMACK received from "<< inet_ntoa(fromAddr.sin_addr)
             << ":" <<ntohs(fromAddr.sin_port) << endl;
    } else {
        cout << "[WARN] No response from gateway." << endl;
    }
    close(sock);
    return 0;}

