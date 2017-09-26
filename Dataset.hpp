#pragma once
#include "Import.hpp"
#include "Logging.hpp"
#include "DataModel.hpp"

class DataModelMNIST
	:public DataModel
{
protected:
	string path_train;
	string path_test;
	
public:
	af::array arr_train_data;
	af::array arr_train_label;
	af::array arr_test_data;
	af::array arr_test_label;

public:
	DataModelMNIST(const string name="MNIST")
	:DataModel(name)
	{
		path_train = ds.base_dir + bearlet_config.configs[ds.name]["train"];
		path_test = ds.base_dir + bearlet_config.configs[ds.name]["test"];

		logout.record() <<"[DataModel] MNIST DataSet.";
		logout.record() <<"[DataModel] Train = " << path_train;
		logout.record() <<"[DataModel] Test = " << path_test;
	}
	
	virtual void save_ff(FormatFile& file)
	{
		FormatLoaderAlignedSeperate loader_train(path_train, ",");
		af::array arr_train;
		loader_train.to_array(arr_train,
				[&](int, int, const string& elem)
				{
					return bearlet_cast<float>(elem);
				}, false);
		arr_train_data = arr_train.cols(1, af::end);
		arr_train_label = arr_train.col(0);

		FormatLoaderAlignedSeperate loader_test(path_test, ",");
		af::array arr_test;
		loader_test.to_array(arr_test,
				[&](int, int, const string& elem)
				{
					return bearlet_cast<float>(elem);
				}, false);
		arr_test_data = arr_test.cols(1, af::end);
		arr_test_label = arr_test.col(0);

		file << arr_train_data;
		file << arr_train_label;
		file << arr_test_data;
		file << arr_test_label;

		logout.record() << "[DataModel] MNIST DataSet Saved.";
	}

	virtual void load_ff(FormatFile& file)
	{
		file >> arr_train_data;
		file >> arr_train_label;
		file >> arr_test_data;
		file >> arr_test_label;

		logout.record() << "[DataModel] MNIST DataSet loaded.";
	}
};

class DataModelWeiboPair
	:public DataModel
{
public:
	string path_post;
	string path_response;
	int max_sentence_size;

public:
	af::array arr_post;
	af::array arr_response;

public:
	map<string, float>	word_namer;
	vector<string>		word_indx;

public:
	DataModelWeiboPair(const string name = "WeiboPair")
	:DataModel(name), max_sentence_size(35)
	{
		word_namer[""] = 0;
		word_indx.push_back("");

		path_post = ds.base_dir + bearlet_config.configs[name]["post"];
		path_response = ds.base_dir + bearlet_config.configs[name]["response"];

		logout.record() << "[DataModel] Post = " << path_post;
		logout.record() << "[DataModel] Response = " << path_response;
	}

public:
	virtual void save_ff(FormatFile& file)
	{
		FormatLoaderUnalignedSeperate loader_post(path_post, max_sentence_size);
		loader_post.to_array(arr_post, 
				[&](int, int, const string& elem)
				{
					if (word_namer.find(elem) == word_namer.end())
					{
						word_namer[elem] = word_namer.size();
						word_indx.push_back(elem);
					}

					return word_namer[elem];
				});
		file << arr_post;

		FormatLoaderUnalignedSeperate loader_response(path_response, max_sentence_size);
		loader_response.to_array(arr_response, 
				[&](int, int, const string& elem)
				{
					if (word_namer.find(elem) == word_namer.end())
					{
						word_namer[elem] = word_namer.size();
						word_indx.push_back(elem);
					}

					return word_namer[elem];
				});
		file << arr_response;

	}

	virtual void load_ff(FormatFile& file)
	{
		file >> arr_post;
		file >> arr_response;
	}
};

class DataModelAbalone
	:public DataModel
{
public:
	string path_data;
	string path_names;

public:
	string descriptions;
	af::array arr_data;

public:
	DataModelAbalone(const string name = "Abalone")
		:DataModel(name)
	{
		path_data = ds.base_dir + bearlet_config.configs[name]["data"];
		path_names = ds.base_dir + bearlet_config.configs[name]["names"];
		
		logout.record() << "[DataModel] Data = " << path_data;
		logout.record() << "[DataModel] Names = " << path_names;
	}

public:
	virtual void save_ff(FormatFile& file)
	{
		FormatLoaderAlignedSeperate loader(path_data, ",");
		loader.to_array(arr_data,
				[&](int, int ifield,const string& elem)
				{
					if (ifield == 0)
					{
						if (elem == "M") return 1.f;
						else if (elem == "F") return -1.f;
						else return 0.f;
					}

					return bearlet_cast<float>(elem);
				}, false);
		file << arr_data;

		FormatFile fin(path_names, ios::in);
		descriptions = fin.read_lines();
		file << descriptions;
	}

	virtual void load_ff(FormatFile& file)
	{
		file >> arr_data;
		file >> descriptions;
	}
};

class DataModelCIFAR
	:public DataModel
{
public:
	af::array arr_train_label;
	af::array arr_train_data;
	af::array arr_test_label;
	af::array arr_test_data;

public:
	string str_class;
	string path_train;
	string path_test;

public:
	int size_label;
	int size_pixel;

public:
	DataModelCIFAR(const string name)
		:DataModel(name), str_class(name)  
	{
		path_train = ds.base_dir + bearlet_config.configs[ds.name]["train"];
		path_test = ds.base_dir + bearlet_config.configs[ds.name]["test"];
		size_label = bearlet_cast<int>(bearlet_config.configs[ds.name]["size_label"]);
		size_pixel = bearlet_cast<int>(bearlet_config.configs[ds.name]["size_pixel"]);
		
		logout.record() << "[DataModel] Train = " << path_train;
		logout.record() << "[DataModel] Test = " << path_test;
		logout.record() << "[DataModel] Size of Label = " << size_label;
		logout.record() << "[DataModel] Size of Pixel = " << size_pixel;
	}

public:
	virtual void save_ff(FormatFile& file)
	{
		af::array arr;

		FormatLoaderByte loader_train(path_train);
		loader_train.to_array<unsigned char>(arr, 
				[&](af::array& arr, vector<unsigned char>& vin)
				{
					int n_line = vin.size()/(size_pixel + size_label);
					logout.record() << "[DataModel] Total Training Samples = " << n_line;

					float* data = new float[vin.size()];
					int iline = 0;
					int ifield = 0;
					for(auto ivin=vin.begin(); ivin != vin.end(); ++ivin)
					{
						data[ifield * n_line + iline] = *ivin;
						++ifield;
						if (ifield == size_label + size_pixel)
						{
							ifield = 0;
							++ iline;
						}
					}

					arr = af::array(n_line, size_pixel + size_label, data); 
					delete[] data;
				});
		arr_train_data = arr.cols(size_label, af::end);
		arr_train_label = arr.cols(0, size_label - 1);

		FormatLoaderByte loader_test(path_test);
		loader_test.to_array<unsigned char>(arr, 
				[&](af::array& arr, vector<unsigned char>& vin)
				{
					int n_line = vin.size()/(size_pixel + size_label);
					logout.record() << "[DataModel] Total Testing Samples = " << n_line;
					
					float* data = new float[vin.size()];
					int iline = 0;
					int ifield = 0;
					for(auto ivin=vin.begin(); ivin != vin.end(); ++ivin)
					{
						data[ifield * n_line + iline] = *ivin;
						++ifield;
						if (ifield == size_label + size_pixel)
						{
							ifield = 0;
							++ iline;
						}
					}
					
					arr = af::array(n_line, size_pixel + size_label, data); 
					delete[] data;
				});
		arr_test_data = arr.cols(size_label, af::end);
		arr_test_label = arr.cols(0, size_label - 1);
		
		file << arr_train_data;
		file << arr_train_label;
		file << arr_test_data;
		file << arr_test_label;

		return;
	}

	virtual void load_ff(FormatFile& file)
	{
		file >> arr_train_data;
		file >> arr_train_label;
		file >> arr_test_data;
		file >> arr_test_label;

		return;
	}
};

class DataModelCIFAR10
	:public DataModelCIFAR
{
public:
	DataModelCIFAR10()
	:DataModelCIFAR("CIFAR.10")
	{

	}
};

class DataModelCIFAR100
	:public DataModelCIFAR
{
public:
	DataModelCIFAR100()
	:DataModelCIFAR("CIFAR.100")
	{

	}
};
