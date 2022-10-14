#pragma once
#include "FasterRandomGenerator.h"
#include "Simulator.h"
#include "Simulator4.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") 


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
