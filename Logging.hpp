#pragma once
#include "Import.hpp"
#include "Config.hpp"
#include "Utils.hpp"
#include "File.hpp"

class Logging
{
protected:
	FormatFile fout;
	
public:
	Logging(const string& base_dir = bearlet_config.configs["Report"]["report_path"])
	:fout(base_dir + time_logging() + ".log", ios::out)
	{
		fout << '[' << time_logging() << ']' << '\t' <<"Ready.";;
		cout << '[' << time_logging() << ']' << '\t' << "Ready.";;
	}

	Logging& record()
	{
		fout << "\n";
		fout << '[' << time_logging() << ']' << '\t';
		cout << "\n";
		cout << '[' << time_logging() << ']' << '\t';

		return *this;
	}

	Logging& record_without_show()
	{
		fout << "\n";
		fout << '[' << time_logging() << ']' << '\t';

		return *this;
	}

public:
	void flush()
	{
		fout.flush();
		cout.flush();
	}

	void redirect(
			const string& str_suffix = "",
			const string& base_dir = bearlet_config.configs["Report"]["report_path"])	
	{
		fout.reset(base_dir + time_logging() + str_suffix + ".log", ios::out);
		
		fout << '[' << time_logging() << ']' << '\t' << "Ready.";
		cout << '[' << time_logging() << ']' << '\t' << "Ready.";
	}

public:
	template<typename T>
	Logging& operator << (T things)
	{
		cout << things;
		fout << things;

		return *this;
	}
};

Logging logout;

class LoggingDatabase
{
protected:
	map<map<string, double>, map<string, double>> result_database;

public:
	LoggingDatabase(const string& base_path = bearlet_config.configs["Report"]["result_database"])
	{
		bearlet_read(base_path,
				[&](FormatFile& file)
				{
					file >> result_database;
				});
	}

public:
	void save(
			const string& base_path = bearlet_config.configs["Report"]["result_database"],
			const string& base_path_pool = bearlet_config.configs["Report"]["result_database_pool"])
	{
		FormatFile file_pool(base_path_pool, ios::binary | ios::app);		
		file_pool << result_database;

		bearlet_write(base_path,
				[&](FormatFile& file)
				{
					file << result_database;
				});
	}

	void restore(
			const string& base_path_pool = bearlet_config.configs["Report"]["result_database_pool"])
	{
		FormatFile file(base_path_pool, ios::binary | ios::in);
		while(!file.end_of_file())
		{
			file >> result_database;
		}

		save();
	}

public:
	void perform(
			function<bool(map<string, double>)> fn_env,
			function<bool(map<string, double>)> fn_result)
	{
		for(	auto ires = result_database.begin();
				ires != result_database.end();
				++ ires)
		{
			if (fn_env(ires->first))
			{
				fn_result(ires->second);
			}
		}
	}

protected:
	map<string, double> current_env;
	map<string, double> current_result;

public:
	void env_push(const string env_name, double env_value)
	{
		current_env[env_name] = env_value;
	}

	void result_push(const string result_name, double result_value)
	{
		current_result[result_name] = result_value;
	}

public:
	void record()
	{
		result_database[current_env] = current_result;
		current_env.clear();
		current_result.clear();
	}
};

LoggingDatabase logdb;
















































