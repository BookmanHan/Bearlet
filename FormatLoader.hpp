#include "Import.hpp"
#include "Logging.hpp"
#include "File.hpp"
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

class FormatLoader
{
protected:
	string loading_content;

public:
	FormatLoader(const string path_name)
	{
		FormatFile file(path_name, ios::in);
		loading_content = file.read_lines(); 

		logout.record() << "[Format Loader] loading from file="<<path_name;
	}
};


class FormatLoaderAlignedSeperate
	:public FormatLoader
{
protected:
	const string segment;

protected:
	vector<string> content_word;

protected:
	int n_lines;
	int n_fields;

public:
	FormatLoaderAlignedSeperate(const string file_name, const string segment)
	:FormatLoader(file_name), segment(segment)
	{
		n_lines = count(loading_content.begin(), loading_content.end(), '\n') - 1;
		boost::split(content_word, loading_content, boost::is_any_of(segment + "\n"));
		
		int n_words = content_word.size();
		n_fields = (int)((float)n_words / (float)n_lines + 0.5); 

		logout.record() << "[Format Loader] Seperate Format.";
		logout.record() << "[Format Loader] Total lines = " << n_lines;
		logout.record() << "[Format Loader] Total fields = " << n_fields;
	}
	
public:
	void to_array(
			af::array& arr_out, 
			function<float(int, int, const string&)> fn_embedding,
			bool should_first_line_be_outfit = false)
	{
		float* data = new float[n_lines * n_fields];
		
		int n_skips = should_first_line_be_outfit?n_fields:0;
		int iline = 0;
		int ifield = 0;
		for(auto i=content_word.begin() + n_skips; i!=content_word.begin() + n_lines * n_fields; ++i)
		{
			data[ifield * n_lines + iline] = fn_embedding(iline, ifield, *i);
			++ ifield;
			if (ifield >= n_fields)
			{
				ifield = 0;
				++ iline;
			}
		}

		arr_out = af::array(n_lines, n_fields, data);
		delete[] data;
	}
};

class FormatLoaderUnalignedSeperate
	:public FormatLoader
{
protected:
	const string segment;
	const string alignment;
	
protected:
	vector<string> content_word;

protected:
	int n_lines;
	int n_fields;

public:
	FormatLoaderUnalignedSeperate(
			const string file_name, 
			const int n_fields,
			const string segment="\t ", 
			const string alignment="$$q234fakxjhekfhuq34uh$$")
	:FormatLoader(file_name), segment(segment), alignment(alignment), n_fields(n_fields)
	{
		n_lines = count(loading_content.begin(), loading_content.end(), '\n') - 1;
		boost::replace_all(loading_content, "\n", " " + alignment + "\n");	
		boost::split(content_word, loading_content, boost::is_any_of(segment + "\n"));

		logout.record() << "[Format Loader] Seperate Format.";
		logout.record() << "[Format Loader] Total lines = " << n_lines;
		logout.record() << "[Format Loader] Total fields = " << n_fields;
	}
	
public:
	void to_array(
			af::array& arr_out, 
			function<float(int, int, const string&)> fn_embedding)
	{
		float* data = new float[n_lines * n_fields];
		fill(data, data + n_lines * n_fields, 0.f);

		int iline = 0;
		int ifield = 0;
		for(auto i=content_word.begin(); i!=content_word.end(); ++i)
		{
			if (*i == alignment)
			{
				ifield = 0;
				++ iline;
				if (iline >= n_lines)
					break;
				continue;
			}
			
			data[ifield * n_lines + iline] = fn_embedding(iline, ifield, *i);
			++ifield;
		}

		arr_out = af::array(n_lines, n_fields, data);
		delete[] data;
	}
};
