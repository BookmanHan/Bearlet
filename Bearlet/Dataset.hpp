#pragma once
#include "Import.hpp"
#include "Logging.hpp"
#include "DataModel.hpp"

class dmMNIST
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
	dmMNIST(const string name="MNIST")
	:DataModel(name)
	{
		path_train = ds.base_dir + bearlet_config.configs[ds.name]["train"];
		path_test = ds.base_dir + bearlet_config.configs[ds.name]["test"];
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
	}

	virtual void load_ff(FormatFile& file)
	{
		file >> arr_train_data;
		file >> arr_train_label;
		file >> arr_test_data;
		file >> arr_test_label;
	}
};

class dmWeiboPair
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
	dmWeiboPair(const string name = "WeiboPair")
	:DataModel(name), max_sentence_size(35)
	{
		word_namer[""] = 0;
		word_indx.push_back("");

		path_post = ds.base_dir + bearlet_config.configs[name]["post"];
		path_response = ds.base_dir + bearlet_config.configs[name]["response"];
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

class dmAbalone
	:public DataModel
{
public:
	string path_data;
	string path_names;

public:
	string descriptions;
	af::array arr_data;

public:
	dmAbalone(const string name = "Abalone")
		:DataModel(name)
	{
		path_data = ds.base_dir + bearlet_config.configs[name]["data"];
		path_names = ds.base_dir + bearlet_config.configs[name]["names"];
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

class dmCIFAR
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
	dmCIFAR(const string name)
		:DataModel(name), str_class(name)  
	{
		path_train = ds.base_dir + bearlet_config.configs[ds.name]["train"];
		path_test = ds.base_dir + bearlet_config.configs[ds.name]["test"];
		size_label = bearlet_cast<int>(bearlet_config.configs[ds.name]["size_label"]);
		size_pixel = bearlet_cast<int>(bearlet_config.configs[ds.name]["size_pixel"]);
		
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

class dmCIFAR10
	:public dmCIFAR
{
public:
	dmCIFAR10()
	:dmCIFAR("CIFAR.10")
	{

	}
};

class dmCIFAR100
	:public dmCIFAR
{
public:
	dmCIFAR100()
	:dmCIFAR("CIFAR.100")
	{

	}
};

class dmSimpleQuestions
	:public DataModel
{
protected:
	string path_FB2M;
	string path_FB5M;
	string path_description;
	string path_train;
	string path_test;
	string path_valid;
	int max_sentence_size;
	
public:
	map<string, int> idx_entity;
	map<string, int> idx_relation;
	af::array arr_FB2M;
	map<string, int> idx_word;
	af::array arr_description_label;
	af::array arr_description_data;
	af::array arr_train_label;
	af::array arr_train_data;
	af::array arr_test_label;
	af::array arr_test_data;
	af::array arr_valid_label;
	af::array arr_valid_data;

public:
	dmSimpleQuestions()
	:DataModel("SimpleQuestions")
	{
		path_FB2M = ds.base_dir + bearlet_config.configs[ds.name]["FB2M"];
		path_FB5M = ds.base_dir + bearlet_config.configs[ds.name]["FB5M"];
		path_description = ds.base_dir + bearlet_config.configs[ds.name]["description"];
		path_train = ds.base_dir + bearlet_config.configs[ds.name]["train"];
		path_test = ds.base_dir + bearlet_config.configs[ds.name]["test"];
		path_valid = ds.base_dir + bearlet_config.configs[ds.name]["valid"];
		max_sentence_size = bearlet_cast<int>(bearlet_config.configs[ds.name]["size_sentence"]);

		logout.record() << "[DataModel] Size of Sentence = " << max_sentence_size;
	}

public:
	virtual void save_ff(FormatFile& file)
	{
		FormatLoaderAlignedSeperate loader_fb2m(path_FB2M, "\t");
		loader_fb2m.to_array(arr_FB2M, 
				[&](int, int ifield, const string& elem)
				{
					switch(ifield)
					{
					case 0:
					case 2:
						if (idx_entity.find(elem) == idx_entity.end())
							idx_entity[elem] = idx_entity.size();
						return idx_entity[elem];
						break;
					case 1:
						if (idx_relation.find(elem) == idx_relation.end())
							idx_relation[elem] = idx_relation.size();
						return idx_relation[elem];
						break;
					default:
						return idx_relation.begin()->second;
						break;
					}
				}, false);

		af::array arr;

//		FormatLoaderUnalignedSeperate loader_description(path_description, 150, "\t ?><[]{}:,!\"\'-");
//		loader_description.to_array(arr, 
//				[&](int, int ifield, const string& elem)
//				{
//					if (ifield == 0 || ifield == 2)
//					{
//						return bearlet_cast<float>(elem);
//					}
//					else if (ifield == 1)
//					{
//						if (idx_entity.find("www.freebase.com" + elem) == idx_entity.end())
//							return -1.f;
//						else
//							return (float)idx_entity["www.freebase.com" + elem];
//					}
//					else
//					{
//						if (idx_word.find(elem) == idx_word.end())
//							idx_word[elem] = idx_word.size();
//						return (float)idx_word[elem];
//					}
//				});
//		arr = arr(arr(af::span, 1) == -1.f, af::span);
//		arr_description_label = arr.col(1);
//		arr_description_data = arr.cols(3, af::end);

		FormatLoaderUnalignedSeperate loader_train(path_train, max_sentence_size, "\t ?><[]{}:,!\"\'-");;
		loader_train.to_array(arr, 
				[&](int, int ifield, const string& elem)
				{
					if (ifield == 0 || ifield == 2)
					{
						return idx_entity[elem];
					}
					else if (ifield == 1)
					{
						return idx_relation[elem];
					}
					else
					{
						if (idx_word.find(elem) == idx_word.end())
							idx_word[elem] = idx_word.size();

						return idx_word[elem];
					}
				});
		arr_train_label = arr.cols(0, 2);
		arr_train_data = arr.cols(3, af::end);
		
		FormatLoaderUnalignedSeperate loader_test(path_test, max_sentence_size, "\t ?><[]{}:,!\"\'-");;
		loader_test.to_array(arr, 
				[&](int, int ifield, const string& elem)
				{
					if (ifield == 0 || ifield == 2)
					{
						return idx_entity[elem];
					}
					else if (ifield == 1)
					{
						return idx_relation[elem];
					}
					else
					{
						if (idx_word.find(elem) == idx_word.end())
							idx_word[elem] = idx_word.size();

						return idx_word[elem];
					}
				});
		arr_test_label = arr.cols(0, 2);
		arr_test_data = arr.cols(3, af::end);
		
		FormatLoaderUnalignedSeperate loader_valid(path_valid, max_sentence_size, "\t ?><[]{}:,!\"\'-");;
		loader_valid.to_array(arr, 
				[&](int, int ifield, const string& elem)
				{
					if (ifield == 0 || ifield == 2)
					{
						return idx_entity[elem];
					}
					else if (ifield == 1)
					{
						return idx_relation[elem];
					}
					else
					{
						if (idx_word.find(elem) == idx_word.end())
							idx_word[elem] = idx_word.size();

						return idx_word[elem];
					}
				});
		arr_valid_label = arr.cols(0, 2);
		arr_valid_data = arr.cols(3, af::end);

		file << idx_entity;
		file << idx_relation;
		file << idx_word;
		file << arr_FB2M;
		file << arr_description_data;
		file << arr_description_label;
		file << arr_train_data;
		file << arr_train_label;
		file << arr_test_data;
		file << arr_test_label;
		file << arr_valid_data;
		file << arr_valid_label;
	}

	virtual void load_ff(FormatFile& file)
	{
		file >> idx_entity;
		file >> idx_relation;
		file >> idx_word;
		file >> arr_FB2M;
		file >> arr_description_data;
		file >> arr_description_label;
		file >> arr_train_data;
		file >> arr_train_label;
		file >> arr_test_data;
		file >> arr_test_label;
		file >> arr_valid_data;
		file >> arr_valid_label;
	}
};

class dmMovielens
	:public DataModel
{
protected:
	string path_rating;
	bool should_first_line_be_outfit;

public:
	af::array arr_rating;

public:
	dmMovielens(const string name)
	:DataModel(name)
	{
		path_rating = ds.base_dir + bearlet_config.configs[ds.name]["rating"];
		should_first_line_be_outfit = bearlet_cast<bool>(bearlet_config.configs[ds.name]["outfit"]);
	}

public:
	virtual void save_ff(FormatFile& file)
	{
		FormatLoaderAlignedSeperate loader(path_rating, ",:");
		loader.to_array(arr_rating, 
				[&](int, int, const string& elem)
				{
					return bearlet_cast<float>(elem);
				}, should_first_line_be_outfit);
		file << arr_rating;
	}

	virtual void load_ff(FormatFile& file)
	{
		file >> arr_rating;
	}
};

class dmMovielens100K
	:public dmMovielens
{
public:
	dmMovielens100K()
		:dmMovielens("Movielens100K")
	{
		;
	}
};

class dmMovielens1M
	:public dmMovielens
{
public:
	dmMovielens1M()
		:dmMovielens("Movielens1M")
	{
		;
	}
};

class dmMovielens10M
	:public dmMovielens
{
public:
	dmMovielens10M()
		:dmMovielens("Movielens10M")
	{
		;
	}
};

class dmMovielens20M
	:public dmMovielens
{
public:
	dmMovielens20M()
		:dmMovielens("Movielens20M")
	{
		;
	}
};

af::array label_vectorization(const af::array& arr, int nfield)
{
	af::array vec_train_label = af::constant(0.f, arr.dims(0), nfield);
	af::array line_pos = af::range(arr.dims(0));
	vec_train_label(line_pos + arr.dims(0) * arr) = 1.f;

	return vec_train_label;
}
