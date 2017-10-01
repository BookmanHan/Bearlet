#pragma once
#include "Import.hpp"
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include "Utils.hpp"

class Storage
{
public:
	static Storage* global_system;
	static Storage* network_system;

public:
	virtual void open_file(
			const string& file_name, 
			const string& file_path, 
			const ios::ios_base::openmode open_mode = ios::binary) = 0;
	virtual void flush_file(const string& file_name) = 0;
	virtual void close_file(const string& file_name) = 0;
	virtual bool end_of_file(const string& file_name) = 0;

public:
	virtual void write(const string file_name, char* p_data, int n_size) = 0;
	virtual void read(const string file_name, char* p_data, int n_size) = 0;
	virtual string read_lines(const string file_name) = 0;
	virtual void read_all(const string file_name, void** data, int& size) = 0;	
};

class DiskStorage
	:public Storage
{
protected:
	map<string, fstream*>	storage_system;

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

	string read_lines(const string file_name)
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found");

		istreambuf_iterator<char> beg(*ifile->second), end;
		string strdata(beg, end);

		return strdata;
	}

	void read_all(const string file_name, void** dout, int& size) 
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found");

		ifile->second->seekg(0, ios::end);
		streampos file_size = ifile->second->tellg();

		char* data = new char[file_size];
		ifile->second->seekg(0, ios::beg);
		ifile->second->read((char*)data, file_size * sizeof(char));

		*dout = data;
		size = file_size;

		return;
	}

public:
	fstream& find_file(const string& file_name)
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found.");

		return *ifile->second;
	}
	
public:
	template<typename T>
	void write(const string& file_name, T elem)
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found.");

		*ifile->second << elem;
	}
	
	template<typename T>
	void read(const string& file_name, T elem)
	{
		auto ifile = storage_system.find(file_name);
		if (ifile == storage_system.end())
			throw string("No File Found.");

		*ifile->second >> elem;
	}
};

class NetworkStorage
	:public Storage
{
protected:
	boost::asio::io_service io;
	map<string, boost::asio::ip::tcp::endpoint> ep;
	map<string, boost::asio::ip::tcp::socket*> sock;

public:
	virtual void open_file(
			const string& file_name, 
			const string& file_path, 
			const ios::ios_base::openmode open_mode = ios::binary)
	{
		vector<string> vstr;
		boost::split(vstr, file_path, boost::is_any_of(":"));

		if (open_mode & ios::in)
		{
			ep[file_name] = boost::asio::ip::tcp::endpoint(
					boost::asio::ip::tcp::v4(), 
					bearlet_cast<int>(vstr[1]));	
			sock[file_name] = new boost::asio::ip::tcp::socket(io);
			boost::asio::ip::tcp::acceptor acc(io, ep[file_name]);
			acc.accept(*sock[file_name]);
		}
		else if (open_mode & ios::out)
		{
			ep[file_name] = boost::asio::ip::tcp::endpoint(
					boost::asio::ip::address::from_string(vstr[0]), 
					bearlet_cast<int>(vstr[1]));	
			sock[file_name] = new boost::asio::ip::tcp::socket(io);
			while(true)
			{	
				try
				{
					sock[file_name]->connect(ep[file_name]);
					break;
				}
				catch(exception& exp)
				{
					cout <<endl << exp.what() <<endl;
				}
			}
		}
	}

	virtual void flush_file(const string&)
	{
		;
	}

	virtual void close_file(const string& file_name)
	{
		sock[file_name]->close();
		delete sock[file_name];
	}
	
	virtual bool end_of_file(const string&)
	{
		return true;
	}

public:
	virtual void write(const string file_name, char* p_data, int n_size)
	{
		while(true)
		{
			int n_write = sock[file_name]->write_some(boost::asio::buffer(p_data, n_size));
			n_size -= n_write;
			p_data += n_write;

			if (n_size == 0)
				break;
		}
	}

	virtual void read(const string file_name, char* p_data, int n_size) 
	{
		while(true)
		{
			int n_read = sock[file_name]->read_some(boost::asio::buffer(p_data, n_size)); 	
			n_size -= n_read;
			p_data += n_read;

			if (n_size == 0)
				break;
		}	
	}
	
	virtual string read_lines(const string)
	{
		throw string("No Implemention.");
	}

	virtual void read_all(const string, void**, int&)
	{
		throw string("No Implemention.");
	}
};

Storage* Storage::global_system = new DiskStorage;
Storage* Storage::network_system = new NetworkStorage;
