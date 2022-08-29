#pragma once
#include <string>
#include <vector>


namespace v3ct0r
{
	// ��������������� �������
	namespace utils
	{
		// ������� ��� ������ �� ��������
		class string final
		{
			public:

				// ��������������� ������ � ����� �� �����
				static std::string format_path(const std::string& path);

				// ������� ���������� ����� �� ���� �����
				static std::string extract_file_ext(const std::string& path);

				// ������� ������ �� �����������
				static void explode(const std::string& in, std::vector<std::string>& out, const char delim);
		};
	};
}
