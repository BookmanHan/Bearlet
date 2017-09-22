#pragma once
#include "Import.hpp"

template<typename T>
class Namer
{
public:
	vector<T>	id_to_name;
	map<T, int>	name_to_id;

public:
	const T & operator [] (const int id)
	{
		return id_to_name[id];
	}

	int operator [] (const T name)
	{
		return name_to_id[id];
	}
};

typedef Namer<string> StringNamer;

inline string time_logging()
{
	const time_t log_time = time(nullptr);
	struct tm* current_time = localtime(&log_time);
	
	stringstream ss;
	ss << 1900 + current_time->tm_year << ".";
	ss << setfill('0') << setw(2) << current_time->tm_mon + 1 << ".";
	ss << setfill('0') << setw(2) << current_time->tm_mday << " ";
	ss << setfill('0') << setw(2) << current_time->tm_hour << ".";
	ss << setfill('0') << setw(2) << current_time->tm_min << ".";
	ss << setfill('0') << setw(2) << current_time->tm_sec;

	return ss.str();
}

template<typename T>
inline
T bearlet_cast(const string str)
{
	stringstream ss(str);

	T value;
	ss >> value;

	return value;
}

template<typename T>
string bearlet_cast(const T elem)
{
	stringstream ss;
	ss << elem;

	return ss.str();
}
