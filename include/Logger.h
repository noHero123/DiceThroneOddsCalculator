#pragma once
#include <iostream>
#include <fstream>
#include <string>

class Logger
{
public:

	Logger(){};

	void setpath(std::string p)
	{
		path = p;
	}

	void logg(std::string s)
	{
		//std::ofstream  myfile(path, std::ios::out | std::ios::app);
		std::ofstream  myfile(path);
		if (myfile.is_open())
		{
			myfile << s << std::endl;
			myfile.close();
		}
	}

	std::string path = "";

};