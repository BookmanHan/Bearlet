#pragma once
#include "Import.hpp"
#include "Logging.hpp"
#include "File.hpp"

class RemoteCommander
{
protected:
	map<string, function<void(FormatFile&)>> fn_proc;
	const string network_path;

public:
	RemoteCommander(const string network_path)
	:network_path(network_path)
	{
		;
	}

public:
	void registe(const string command_name, function<void(FormatFile&)> fn_parameter)
	{
		fn_proc[command_name] = fn_parameter;
	}

public:
	void send(const string command_name, function<void(FormatFile&)> fn_parameter)
	{
		FormatFile file(network_path, ios::out, sNetwork);
		file << command_name;
		fn_parameter(file);
	}

	void recieve()
	{
		FormatFile file(network_path, ios::in, sNetwork);
		string command_name;
		file >> command_name;

		fn_proc[command_name](file);
	}
};

