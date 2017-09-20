#pragma once
#include "Import.hpp"
#include "Storage.hpp"

class FormatFile
{
public:
	Storage& stg;
	string	name;

public:
	FormatFile(
		const string name,
		const string file_path,
		const ios::ios_base::openmode open_mode,
		Storage& stg = *Storage::global_system)
		:stg(stg), name(name)
	{
		stg.open_file(name, file_path, open_mode);
	}

	FormatFile(
		const string file_path,
		const ios::ios_base::openmode open_mode,
		Storage& stg = *Storage::global_system)
		:stg(stg), name(file_path)
	{
		stg.open_file(name, file_path, open_mode);
	}

public:
	bool end_of_file()
	{
		return stg.end_of_file(name);
	}

	void flush()
	{
		return stg.flush_file(name);
	}

	void close()
	{
		stg.close_file(name);
		delete this;
	}

	void try_close()
	{
		stg.close_file(name);
	}

	virtual ~FormatFile()
	{
		try_close();
	}

	virtual void reset(
		const string file_path,
		const ios::ios_base::openmode open_mode)
	{
		try_close();
		name = file_path;
		stg.open_file(name, file_path, open_mode);
	}

public:
	FormatFile& write(char* p_data, int n_size)
	{
		stg.write(name, p_data, n_size);
		return *this;
	}

	FormatFile& read(char* p_data, int n_size)
	{
		stg.read(name, p_data, n_size);
		return *this;
	}
};

class FormatLoad
	:public FormatFile
{
public:
	FormatLoad(const string name, const string file_path)
		:FormatFile(name, file_path, ios::binary | ios::in)
	{
		char word_size = sizeof(int);
		read((char*)&word_size, sizeof(char));

		if (word_size != sizeof(int))
			throw string("Address Model Unmatched.");
	}

	FormatLoad(const string file_path)
		:FormatFile(file_path, ios::binary | ios::in)
	{
		char word_size = sizeof(int);
		read((char*)&word_size, sizeof(char));

		if (word_size != sizeof(int))
			throw string("Address Model Unmatched.");
	}

	virtual ~FormatLoad()
	{
		;
	}

protected:
	friend FormatFile& make_fout(const string file_path);
	friend FormatFile& make_fin(const string file_path);
};

class FormatSave
	:public FormatFile
{
public:
	FormatSave(const string name, const string file_path)
		:FormatFile(name, file_path, ios::binary | ios::out)
	{
		char word_size = sizeof(int);
		write((char*)&word_size, sizeof(char));
	}

	FormatSave(const string file_path)
		:FormatFile(file_path, ios::binary | ios::out)
	{
		char word_size = sizeof(int);
		write((char*)&word_size, sizeof(char));
	}

	virtual ~FormatSave()
	{
		;
	}

protected:
	friend FormatFile& make_fout(const string file_path);
	friend FormatFile& make_fin(const string file_path);
};

inline
FormatFile& make_fout(const string file_path)
{
	return *(new FormatSave(file_path));
}


inline
FormatFile& make_fin(const string file_path)
{
	return *(new FormatLoad(file_path));
}

inline
void make_close(FormatFile& that)
{
	delete &that;
}

void bearlet_read(const string& file_path, function<void(FormatFile&)> fn_proc)
{
	FormatFile& fout = make_fin(file_path);
	fn_proc(fout);
	make_close(fout);

	return;
}

void bearlet_write(const string& file_path, function<void(FormatFile&)> fn_proc)
{
	FormatFile& fin = make_fout(file_path);
	fn_proc(fin);
	make_close(fin);

	return;
}

template<typename T>
inline
FormatFile& operator << (FormatFile& file, const T& src)
{
	file.write((char*)&src, sizeof(T));

	return file;
}

template<typename T>
inline
FormatFile& operator >> (FormatFile& file, T& src)
{
	file.read((char*)&src, sizeof(T));

	return file;
}

inline
FormatFile& operator << (FormatFile& file, const string& src)
{
	auto n_len = src.length();
	file.write((char*)&n_len, sizeof(n_len));
	file.write((char*)src.data(), sizeof(char)*n_len);

	return file;
}

inline
FormatFile& operator >> (FormatFile& file, string& src)
{
	decltype(src.length()) n_len;
	file.read((char*)&n_len, sizeof(n_len));

	char* ca_str = new char[n_len + 1];
	file.read((char*)ca_str, sizeof(char)*n_len);
	ca_str[n_len] = 0;
	src = ca_str;

	delete[] ca_str;

	return file;
}

template<typename T>
inline
FormatFile& operator << (FormatFile& file, const vector<T>& src)
{
	int size_src = src.size();
	file.write((char*)&size_src, sizeof(int));

	for (auto i = src.begin(); i != src.end(); ++i)
	{
		file << (*i);
	}

	return file;
}

template<typename T>
inline
FormatFile& operator >> (FormatFile& file, vector<T>& src)
{
	int size_src = src.size();
	file.read((char*)&size_src, sizeof(int));

	src.resize(size_src);
	for (auto i = src.begin(); i != src.end(); ++i)
	{
		file >> (*i);
	}

	return file;
}

template<typename T>
inline
FormatFile& operator << (FormatFile& file, const set<T>& src)
{
	int size_src = src.size();
	file.write((char*)&size_src, sizeof(int));

	for (auto i = src.begin(); i != src.end(); ++i)
	{
		file << (*i);
	}

	return file;
}

template<typename T>
inline
FormatFile& operator >> (FormatFile& file, set<T>& src)
{
	int size_src = src.size();
	file.read((char*)&size_src, sizeof(int));

	for (auto i = 0; i != size_src; ++i)
	{
		T tmp;
		file >> tmp;

		src.insert(tmp);
	}

	return file;
}

template<typename T_first, typename T_second>
inline
FormatFile& operator << (FormatFile& file, const map<T_first, T_second>& src)
{
	int size_src = src.size();
	file.write((char*)&size_src, sizeof(int));

	for (auto i = src.begin(); i != src.end(); ++i)
	{
		file << i->first;
		file << i->second;
	}

	return file;
}

template<typename T_first, typename T_second>
inline
FormatFile& operator >> (FormatFile& file, map<T_first, T_second>& src)
{
	int size_src = src.size();
	file.read((char*)&size_src, sizeof(int));

	for (auto i = 0; i != size_src; ++i)
	{
		pair<T_first, T_second> tmp;
		file >> tmp.first;
		file >> tmp.second;

		src.insert(tmp);
	}

	return file;
}
