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
	}

public:
	virtual string get_string() = 0;
};


class FormatLoaderCSV
	:public FormatLoader
{
protected:
	const string segment;

protected:
	vector<string> content_lines;
	int n_current_line;

protected:
	vector<string> vstr_current_line;
	int n_current_word;

public:
	FormatLoaderCSV(const string file_name, const string segment)
	:FormatLoader(file_name), segment(segment), n_current_line(0), n_current_word(10000000) 
	{
		boost::split(content_lines, loading_content, boost::is_any_of("\n\r"));
		for(auto i=content_lines.begin(); i!=content_lines.end(); ++i)
		{
			boost::trim(*i);
			if (i->length() <= 1)
			{
				content_lines.erase(i, content_lines.end());
				break;
			}
		}
	}
	
public:
	virtual string get_string()
	{
		++ n_current_word;
		if (n_current_word >= (int)vstr_current_line.size())
		{
			vstr_current_line.clear();
			boost::split(vstr_current_line, content_lines[n_current_line], boost::is_any_of(segment));
			++n_current_line;
			n_current_word = 0;
		}

		return vstr_current_line[n_current_word];
	}

public:
	int n_line;
	int n_filed;
	vector<string> str_outfit;

public:
	void deal_with_table_outfit(bool should_first_line_be_outfit = true)
	{
		n_line = content_lines.size() - 1;
		get_string();
		str_outfit = vstr_current_line;
		n_filed = vstr_current_line.size();

		n_current_word = 10000000;

		if (should_first_line_be_outfit)
			++n_line;
		else
			n_current_line = 1;
	}

public:
	void to_array(
			af::array& arr_out, 
			function<float(int, int, const string)> fn_embedding,
			bool should_first_line_be_outfit)
	{
		deal_with_table_outfit(should_first_line_be_outfit);
		
		float* data = new float[n_line * n_filed];
		for(auto iline = 0; iline < n_line; ++iline)
		{
			for(auto ifield=0; ifield < n_filed; ++ifield)
			{
				string str_out = get_string();
				data[ifield * n_line + iline] = fn_embedding(iline, ifield, str_out); 
			}
		}

		arr_out = af::array(n_line, n_filed, data);
		delete[] data;
	}
};
