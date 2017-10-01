#pragma once
#include "Import.hpp"

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
	stringstream ss;
	ss << str;
	T value;
	ss >> value;
	ss.str("");

	return value;
}

template<typename T>
string bearlet_cast(const T elem)
{
	static stringstream ss;
	ss.str("");
	ss << elem;

	return ss.str();
}

inline
char print_array(const af::array& _)
{
	af_print(_);
	return ' ';
}
