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

class Plot
{
public:
	RemoteCommander cmd;

public:
	Plot(const string network_path)
	:cmd(network_path)
	{
		;
	}

public:
	void start()
	{
		af::Window window;
		
		cmd.registe("setSize", 
				[&](FormatFile& file)
				{
					int w, h;
					file >> w >>h;
					window.setSize(w, h);
				});	
		cmd.registe("plot", 
				[&](FormatFile& file)
				{
					af::array arr;
					string str;
					file >> arr >> str;
					window.plot(arr, str.c_str());
				});

		thread thd_recieve([&](){while(true) cmd.recieve();});
		do
		{
			window.show();
		} while(!window.close());
	}

public:
	void setSize(int w, int h)
	{
		cmd.send("setSize", 
				[&](FormatFile& file)
				{
					file << w << h;
				});
	}

	void plot(const af::array& arr, string title)
	{
		cmd.send("plot", 
				[&](FormatFile& file)
				{
					file << arr << title;
				});
	}
};
