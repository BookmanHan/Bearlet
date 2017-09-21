#pragma once
#include "Import.hpp"
#include "Config.hpp"
#include "Utils.hpp"
#include "File.hpp"

class Logging
{
protected:
	FormatLog fout;
	
public:
	Logging(const string& base_dir = bearlet_config.configs["Report"]["report_path"])
	:fout(base_dir + time_logging() + ".log")
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

	~Logging()
	{
		flush();
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

	FormatFile& direct()
	{
		return fout;
	}
};

Logging logout;

class LoggingDatabase
{
public:
	vector<pair<map<string, double>, map<string, double>>> db_results;

public:
	LoggingDatabase(
			const string path_db_result = bearlet_config.configs["Report"]["result_database"])
	{
		try
		{
			FormatFile file(path_db_result, ios::in | ios::binary);
			file >> db_results;
		}
		catch(string& str)
		{
			logout.record() << "[Logging Database] Fatal Error for Result Database: " << str; 
		}
	}

	~LoggingDatabase()
	{
		save();
	}

public:
	void save(
			const string path_db_result = bearlet_config.configs["Report"]["result_database"],
			const string path_db_pool = bearlet_config.configs["Report"]["result_database_pool"])
	{
		try
		{
			FormatFile file(path_db_result, ios::binary | ios::out);
			file << db_results;
		}
		catch(string& str)
		{
			logout.record() << "[Logging Database] Fatal Error for Saving Result Database: " << str;
		}

		try
		{
			FormatFile file(path_db_pool, ios::binary | ios::app);
			file << db_results;
		}
		catch(string& str)
		{
			logout.record() << "[Logging Database] Fatal Error for Saving Result Pool: " << str;
		}
	}

	void restore(
			const string path_db_result = bearlet_config.configs["Report"]["result_database"],
			const string path_db_pool = bearlet_config.configs["Report"]["result_database_pool"])
	{
		db_results.clear();

		try
		{
			FormatFile file(path_db_pool, ios::binary | ios::in);
			try
			{
				while(!file.end_of_file())
				{
					file >> db_results;
				}
			}
			catch(string& str)
			{
				logout.record() << "[Logging Database] Error for Restroing Result Database: " << str;
			}

			if (file.end_of_file())
				logout.record() << "[Logging Database] Sucessfully Restored";
			else
				logout.record() << "[Logging Database] Unsucessfully Restored, Good Luck.";
		}
		catch(string& str)
		{
			logout.record() << "[Logging Database] Fatal Error for Restoring Result Database: " << str;
		}
	}

protected:
	map<string, double> current_env;
	map<string, double> current_result;

public:
	void push_env(const string name, double value)
	{
		current_env[name] = value;
	}

	void push_result(const string name, double value)
	{
		current_result[name] = value;
	}

	void record()
	{
		db_results.push_back(make_pair(current_env, current_result));
		current_env.clear();
		current_result.clear();
	}

public:
	void perform(
			function<bool(map<string, double>&)> fn_env,
			function<bool(map<string, double>&)> fn_result)
	{
		for(auto i=db_results.begin(); i!=db_results.end(); ++i)
		{
			if (fn_env(i->first))
				fn_result(i->second);
		}
	}
};

LoggingDatabase logdb;
