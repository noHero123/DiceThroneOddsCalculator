#include <future>
#include <iostream>

#include "Types.h"
#include "Server.h"
#include "json.h" //https://github.com/nlohmann/json

using namespace std;
using json = nlohmann::json;

void DTServer::send_to_client_test()
{
	json jresponse;
	jresponse["messageID"] = 2;
	jresponse["customMessage"] = { {"test1", "test2"}, {"test3", 42.99} };
	std::string response_message = jresponse.dump();
	send_to_client(response_message, 39999);
}

void DTServer::send_to_client(std::string message, int port)
{
	//cout << "send:"<<endl << message << endl;
	WSADATA wsa_data;
	SOCKADDR_IN addr;

	WSAStartup(MAKEWORD(2, 2), &wsa_data);
	const auto sender = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	auto blah = InetPton(AF_INET, (PCWSTR)("127.0.0.1"), &addr.sin_addr.s_addr);
	//cout << " port "<< port << endl;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	//cout << "connect"<< endl;
	if (connect(sender, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)))
	{
		//cout << "send" << endl;
		send(sender, message.c_str(), message.length(), 0);
		
	}
	closesocket(sender);
	WSACleanup();
	//cout << "Socket closed." << endl << endl;
}

std::vector<std::string> string_split(std::string message, std::string delimiter)
{
	std::vector<std::string> result{};
	size_t pos = 0;
	std::string s = message;
	std::string token = "";
	std::string content_lengt = "";
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		result.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	result.push_back(s);
	return result;
}

std::string urlDecode(std::string& SRC) {
	std::string ret;
	char ch;
	int i, ii;
	for (i = 0; i < SRC.length(); i++) {
		if (SRC[i] == '%') {
			sscanf_s(SRC.substr(i + 1, 2).c_str(), "%x", &ii);
			ch = static_cast<char>(ii);
			ret += ch;
			i = i + 2;
		}
		else {
			ret += SRC[i];
		}
	}
	return (ret);
}

void DTServer::on_client_connect(SOCKET client)
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));

	cout << "Client connected!" << endl;
	int result = recv(client, buffer, sizeof(buffer), 0);
	std::string message_str = "";
	std::string message_str2(buffer, result);
	message_str += message_str2;

	auto splits = string_split(message_str, "\r\n");
	string s = splits[splits.size() - 1];
	if (s.size() == 0)
	{
		result = recv(client, buffer, sizeof(buffer), 0);
		std::string message_str3(buffer, result);
		message_str += message_str3;
		s = message_str3;
	}
	std::cout << "client says " << message_str << std::endl;
	std::cout << "client says " << s << std::endl;
	auto client_data = string_split(s, "&");
	std::string hero_name = "";
	size_t sixit = 0;
	size_t tipit = 0;
	size_t samesis = 0;
	size_t wild = 0;
	size_t twiceWild = 0;
	size_t slightlyWild = 0;
	size_t cp = 0;
	size_t anzcards = 0;
	size_t rollattempts = 0;
	size_t rerolls = 0;
	bool last_die_is_scarlett = false;
	bool is_default_sim = false;
	bool is_chase = false;
	std::string chase_ability = "";
	bool scarlett_die = false;
	std::string dice_anatomy = "";
	std::vector<std::string> abilities = {};

	std::vector<DiceIdx> dice{0,0,0,0,0};

	for (auto pair : client_data)
	{
		auto key_value = string_split(pair, "=");
		if (key_value.size() < 2)
		{
			continue;
		}
		std::string key = key_value[0];
		std::string value = key_value[1];
		value = urlDecode(value);
		std::cout << key << " " << value << std::endl;
		if (key == "hero")
		{
			hero_name = value;
		}
		if (key == "dice1")
		{
			dice[0] = max(std::stoi(value) - 1, 0);
		}
		if (key == "dice2")
		{
			dice[1] = max(std::stoi(value) - 1, 0);
		}
		if (key == "dice3")
		{
			dice[2] = max(std::stoi(value) - 1, 0);
		}
		if (key == "dice4")
		{
			dice[3] = max(std::stoi(value) - 1, 0);
		}
		if (key == "dice5")
		{
			dice[4] = max(std::stoi(value) - 1, 0);
		}
		if (key == "sixit")
		{
			sixit = std::stoi(value);
		}
		if (key == "tipit")
		{
			tipit = std::stoi(value);
		}
		if (key == "samesis")
		{
			samesis = std::stoi(value);
		}
		if (key == "wild")
		{
			wild = std::stoi(value);
		}
		if (key == "twicewild")
		{
			twiceWild = std::stoi(value);
		}
		if (key == "slightlywild")
		{
			slightlyWild = std::stoi(value);
		}
		if (key == "cp")
		{
			cp = std::stoi(value);
		}
		if (key == "maxcards")
		{
			anzcards = std::stoi(value);
		}
	}

	std::vector<OddsResult> ergs;
	//ergs = simulator.get_default_probability(hero_name, dice, sixit, samesis, tipit, wild, twiceWild, slightlyWild, cp, anzcards);
	// normal chase all variant:
	if (scarlett_die)
	{
		ergs = simulator4_.get_probability(hero_name, abilities, dice_anatomy, is_default_sim, is_chase, chase_ability, dice, sixit, samesis, tipit, wild, twiceWild, slightlyWild, cp, anzcards, rollattempts, rerolls);
	}
	else
	{
		ergs = simulator_.get_probability(hero_name, abilities, dice_anatomy, is_default_sim, is_chase, chase_ability, dice, sixit, samesis, tipit, wild, twiceWild, slightlyWild, cp, anzcards, rollattempts, rerolls);
	}
	

	json jresponse;
	jresponse["messageID"] = 2;
	for (const auto & erg : ergs)
	{
		jresponse["customMessage"][erg.ability]["odd"] = erg.get_odds();
		jresponse["customMessage"][erg.ability]["reroll"] = erg.get_reroll();
	}
	std::string response_message = jresponse.dump();
	int size_byte = response_message.size() * sizeof(std::string::value_type);
	std::string myHeader = "HTTP/1.1 200 OK\r\n";
	//myHeader += "Server: Apache / 1.3.29 (Unix)PHP / 4.3.4\r\n";
	myHeader += "Connection: keep-alive\r\n";
	myHeader += "Content-Type: application/json; charset = utf-8\r\n";
	myHeader += "Content-Length: " + std::to_string(size_byte) + "\r\n";

	response_message = myHeader +"\r\n" + response_message;
	std::cout <<"send to client:"<<std::endl << response_message << std::endl;
	send(client, response_message.c_str(), response_message.length(), 0);
	closesocket(client);
}

DTServer::DTServer(int port, DiceRoller& helper) : simulator_{helper}, simulator4_{ helper }
{
	WSADATA wsa_data;
	SOCKADDR_IN server_addr, client_addr;

	WSAStartup(MAKEWORD(2, 2), &wsa_data);
	const auto server = socket(AF_INET, SOCK_STREAM, 0);

	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	::bind(server, reinterpret_cast<SOCKADDR*>(&server_addr), sizeof(server_addr));
	listen(server, 0);

	cout << "Listening for incoming connections..." << endl;

	int client_addr_size = sizeof(client_addr);

	for (;;)
	{
		SOCKET client;

		if ((client = accept(server, reinterpret_cast<SOCKADDR*>(&client_addr), &client_addr_size)) != INVALID_SOCKET)
		{
			//auto fut = async(launch::async, &DTServer::on_client_connect39998, this, client);
			auto fut = async(launch::async, &DTServer::on_client_connect, this, client);
		}

		const auto last_error = WSAGetLastError();

		if (last_error > 0)
		{
			cout << "Error: " << last_error << endl;
		}
	}
}