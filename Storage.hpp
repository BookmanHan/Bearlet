#pragma once
#include "Import.hpp"

class Storage
{
protected:
	map<string, fstream*>	storage_system;

public:
	static Storage* global_system;

public:
	void open_file(const string& file_name, const string& file_path, const ios::ios_base::openmode open_mode = ios::binary)
	{
		storage_system[file_name] = new fstream(file_path.c_str(), open_mode);
		if (storage_system[file_name]->bad() || storage_system[file_name]->fail())
			throw string("Bad File Path.");
	}

	void flush_file(const string& file_name)
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found.");

		ifile->second->flush();
	}

	void close_file(const string& file_name)
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found.");

		ifile->second->close();
		storage_system.erase(ifile);
	}

	bool end_of_file(const string& file_name)
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found.");

		return ifile->second->eof();
	}

public:
	void write(const string file_name, char* p_data, int n_size)
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found");

		ifile->second->write(p_data, n_size);
	}
	
	void read(const string file_name, char* p_data, int n_size)
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found");

		ifile->second->read(p_data, n_size);
	}

	template<typename T>
	void write(const string file_name, const T& elem)
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found");

		*(ifile->second) << elem;
	}

	template<typename T>
	void read(const string file_name, const T& elem)
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found");

		*(ifile->second) >> elem;
	}

public:
	fstream& find_file(const string& file_name)
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found.");

		return *ifile->second;
	}
};

Storage* Storage::global_system = new Storage;
