#pragma once
#include "Import.hpp"
#include "Logging.hpp"
#include "File.hpp"

class RemoteCommander
{
protected:
	map<string, function<void(FormatFile&)>> fn_proc;
	FormatFile* file;

public:
	RemoteCommander(const string network_path, ios::openmode open_mode)
	{
		file = new FormatFile(network_path, open_mode, sNetwork);
	}

	~RemoteCommander()
	{
		delete file;
	}

public:
	void registe(const string command_name, function<void(FormatFile&)> fn_parameter)
	{
		fn_proc[command_name] = fn_parameter;
	}

public:
	void send(const string command_name, function<void(FormatFile&)> fn_parameter)
	{
		(*file) << command_name;
		fn_parameter(*file);
	}

	void recieve()
	{
		string command_name;
		(*file) >> command_name;

		fn_proc[command_name](*file);
	}
};
