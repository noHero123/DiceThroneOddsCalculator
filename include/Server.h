#pragma once
#include "FasterRandomGenerator.h"
#include "Simulator.h"
#include "Simulator4.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") 
//#else
//#include <experimental/filesystem> // older gcc
#endif

#if defined(__unix__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int SOCKET;
#define closesocket(i) close(i)
#define ioctlsocket(i,l,ul) ioctl(i,l,ul)
#define SOCKADDR sockaddr
#define SOCKADDR_IN sockaddr_in
#define INVALID_SOCKET -1
#endif





class DTServer
{
public:

	DTServer(int port, DiceRoller& drr);

	void on_client_connect(SOCKET client);
	void static send_to_client(std::string message, int port);
	//void on_client_connect39998(SOCKET client);
	void static send_to_client_test();

	Simulator simulator_;
	Simulator4 simulator4_;


};
