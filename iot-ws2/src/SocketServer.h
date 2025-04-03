#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include "MessageHandler.h"

/*
 Simple UDP server
 -recieves messages from ebikes clients
 -handle JOIN,DATA, COMMAD and send appropriate message("OK" or "COMMACK")
 */
class SocketServer
{
public:
    /*
     constructor that sets the listening port(8080 in our case)
     */
    SocketServer(int port) : port(port) {}

    /*
     starts the udp server and recieves and handles messages.
     */
    void start()
    {
        //create a UDP socket
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            perror("Socket creation failed");
            return;
        }

        //configure the server address
        struct sockaddr_in serverAddr;
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);  // Change IP if needed
        serverAddr.sin_port = htons(port);

        // bindthe socket to a given IP and port
        if (bind(sock, reinterpret_cast<const struct sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
            perror("Bind failed");
            close(sock);
            return;
        }

        std::cout << "UDP Socket Server listening on port " << port << std::endl;

        char buffer[1024]; // buffer which we will store received message in it

        while(true)
        {
            struct sockaddr_in clientAddr;
            socklen_t len= sizeof(clientAddr);

            //receive UDP message from client
            int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,reinterpret_cast<struct sockaddr*>(&clientAddr), &len);
            if (n > 0) {
                buffer[n] ='\0';
                std::string message(buffer);

                //identify the type of message the using its "directive"..
                std::string directive = MessageHandler::extractDirective(message);

                if (directive =="COMMAND") {
                    /*COMMAND message:update lock status and reply with COMMACK*/
                    MessageHandler::handleMessage(message);
                    std::string commack = MessageHandler::buildCOMMACK();
                    sendto(sock, commack.c_str(), commack.size(), 0,reinterpret_cast<struct sockaddr*>(&clientAddr), len);
                } else {
                    //JOIN or DATA message: process and respond with OK
                    MessageHandler::handleMessage(message);
                    std::string ack= "OK";
                    sendto(sock, ack.c_str(), ack.size(), 0,reinterpret_cast<struct sockaddr*>(&clientAddr), len);
                }}
        }

        close(sock); // exit socket
    }

private:
    int port; // data type(the port number)
};

#endif

