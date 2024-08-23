#pragma once


#include <string>

class ClientHandler {
public:
    static void Init();

    static void Start();

private:
    static void AcceptConnections();
    static void ReceiveData();
    static void ProcessData();
    static void SendData();
};
