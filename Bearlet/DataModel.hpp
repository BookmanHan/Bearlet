#pragma once
#include "Import.hpp"
#include "Logging.hpp"
#include "Dataset.hpp"
#include "FormatLoader.hpp"

class DataSet
{
public:
	const string name;
public:
	string base_dir;
	string path_bindata;

public:
	DataSet(const string name)
		:name(name)
	{
		 base_dir = bearlet_config.configs[name]["base_dir"];
		 path_bindata = bearlet_config.configs[name]["bldata"];
	}
};

class DataModel
{
public:
	DataSet ds;

public:
	DataModel(const string name)
		:ds(name)
	{
		logout.record() << "[DataModel] loading DataSet " << ds.name << ".";
	}

public:
	virtual void load_ff(FormatFile& file) = 0;

	void load(const string file_name)
	{
		logout.record() <<"[DataModel] bldata = " <<file_name;
		bearlet_read(file_name,
				[&](FormatFile& file)
				{
					load_ff(file);
				});
		logout.record() <<"[DataModel] DataSet " << ds.name << " loaded.";
	}
	
	void load()
	{
		load(ds.base_dir + ds.path_bindata);
	}

public:
	virtual void save_ff(FormatFile& file) = 0;
	
	void save(const string file_name)
	{
		logout.record() <<"[DataModel] bldata = " <<file_name;
		bearlet_write(file_name,
				[&](FormatFile& file)
				{
					save_ff(file);
				});
		logout.record() <<"[DataModel] DataSet " << ds.name << " saved.";
	}
	
	void save()
	{
		save(ds.base_dir + ds.path_bindata);
	}
};
