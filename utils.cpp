#include "utils.h"
#include <algorithm>


namespace v3ct0r
{
	// ��������������� ������ � ����� �� �����
	std::string utils::string::format_path(const std::string& path)
	{
		// ����� �������� ������
		std::string copy(path);

		// ������� ����� �� ��������
		std::replace(copy.begin(), copy.end(), '/', '\\');

		// ������� �������� ���� � �����
		if (copy.empty() == false && copy.back() != '\\') {
			copy.push_back('\\');
		}

		return copy;
	}

	// ������� ���������� ����� �� ���� �����
	std::string utils::string::extract_file_ext(const std::string& path)
	{
		const std::string::size_type pos = path.rfind('.');

		if (pos == std::string::npos) {
			return "";
		}

		return path.substr(pos + 1);
	}

	// ������� ������ �� �����������
	void utils::string::explode(const std::string& in, std::vector<std::string>& out, const char delim)
	{
		// ��������, ��� ������� ������ �� �����
		if (in.empty() == true) {
			return;
		}

		// ������� � ��������� ������� ������ �����������
		std::size_t pos = 0, start = 0;

		// Loop over the string
		while ( (pos = in.find(delim, pos)) != std::string::npos )
		{
			// ��������� ��������� � ������
			out.push_back( in.substr(start, pos - start) );

			// ������������� ����� ��������� ������� ������ �����������
			start = ++pos;
		} 
	
		// ��������� � ������ ��������� ���������
		out.push_back( in.substr(start, pos - start) );
	}
}