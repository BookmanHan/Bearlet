#pragma once
#include "Import.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

class Config
{
public:
	map<string, map<string, string>> configs;

public:
	Config(const string& filename)
	{
		boost::property_tree::ptree root;
		boost::property_tree::read_ini(filename, root);
		for(auto i=root.begin(); i!=root.end(); ++i)
		{
			map<string, string> subconfigs;
			for(auto j=i->second.begin(); j!=i->second.end(); ++j)
			{
				subconfigs.insert(make_pair(j->first, j->second.get_value<string>()));
			}
			configs.insert(make_pair(i->first, subconfigs));
		}
	}
};

Config bearlet_config("application.blconfig");
