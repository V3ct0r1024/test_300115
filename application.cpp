#include "application.h"
#include "utils.h"

#include <vector>
#include <thread>
#include <functional>

#include <Windows.h>


namespace v3ct0r
{
	// �����������
	application::application() :
		m_output(nullptr)
	{

	}

	// ��������� ���������
	application::return_errno_t application::run(int argc, char** argv)
	{
		// �������� ����� ����������
		if (argc != 3) {
			return return_errno_t::wrong_params;
		}

		// ������������� ���� �� ���������� c �������
		const std::string input_path = utils::string::format_path(argv[1]);

		// ������� ���� �� ��������� �����
		const std::string output_path = std::string(argv[2]);

		// ������ ������ ������ ������
		file_list_t().swap(m_files);

		// ���� ����� � ��������� ����������
		search_files(input_path, m_files);

		// ��������, ��� ����� �� �������
		if (m_files.empty() == true)
		{
			// ������� �� ���������
			return return_errno_t::no_files_found;
		}

		// ��������� �������� ���� �� ������
		m_output = std::make_shared<std::ofstream>(output_path, std::ofstream::out | std::ofstream::trunc);

		// ��������, ��� ���� ������
		if (m_output->is_open() == false)
		{
			// �� ������� �������/������� �������� ����
			return return_errno_t::output_error;
		}

		// ������� ����� ����������� � �������
		const unsigned int processor_count = max(std::thread::hardware_concurrency(), 1);

		// ������ ������� �������
		std::vector<std::thread> workers;
		
		// ��������� processor_count - 1 ������� �������
		for (unsigned int i = 0; i < (processor_count - 1); ++i)
		{
			// ������� ����� . . .
			std::thread th( std::bind( std::mem_fn(&application::work), this ) );

			// . . . � ��������� ��� � ������
			workers.push_back( std::move(th) );
		}

		// ������������ ����� ����� �������� �������
		work();

		// ���� ����� ������� ������ �������� ����������
		std::for_each(workers.begin(), workers.end(), [](std::thread& t)
		{
			t.join();
		});

		// ��������� �������� ����
		m_output->close();

		// ����� ����������
		return return_errno_t::success;
	}

	// ����� ��������� ����� � ��������� ����������
	void application::search_files(const std::string& path, file_list_t& files)
	{
		// ��������� ����, ��������� ��������� FindFirstFile, FindFirstFileEx, ��� FindNextFile
		WIN32_FIND_DATAA FindFileData;

		/* ��������� ���������� ������ � ���������� ���������� � ������ �����,
				� ������, ������� �������� �������� �������� */
		HANDLE hFind = FindFirstFileA((path + "*.*").c_str(), &FindFileData);

		// ��������, ��� ������ ���� �� ���� ����
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				// ������� ��� �������� �����
				const std::string file_name(FindFileData.cFileName);

				// �������� ������� ����� - ����������
				if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (file_name != "." && file_name != "..")
					{
						// ���������� ���� ����� � �������������
						search_files((path + file_name + "\\"), files);
					}
				}
				else
				{
					// ��������� ��������� ��������� ���� � ������
					if (utils::string::extract_file_ext(file_name) == "txt")
					{
						files.push(path + file_name);
					}
				}
			} while (FindNextFileA(hFind, &FindFileData) != FALSE);
		}

		// ��������� ���������� ������
		FindClose(hFind);
	}

	// �������� ��������� �������� ������
	void application::work()
	{
		// ����� ���� �� ���������� ����� �� �������
		auto take_path = [&mtx = m_files_mtx, &files = m_files](std::string& out) -> bool
		{
			// ��������� ������ ������
			std::unique_lock<std::mutex> lock(mtx);

			// ��������, ��� � ������ ���� �����
			if (files.empty() == true)
			{
				// � ������ �� �������� ������
				return false;
			}

			// ����� ����
			const std::string path = files.front();

			// ������� ���� �� ������
			files.pop();

			// �������� ���� �� ����� �� ������
			out = path;
	
			return true;
		};

		// ������ ����� �� �������� �����
		std::vector<std::string> body;
		body.reserve(16);

		// ������ ��������
		std::vector<std::string> substrings;
		substrings.reserve(256);

		// ������������ ����� . . .
		for (std::string cur_path(""); take_path(cur_path) == true; )
		{
			// ��������� ��������� ����
			std::ifstream file(cur_path, std::ifstream::in);

			// ��������, ��� ���� ������
			if (file.is_open() == false)
			{
				// ���������� ����
				continue;
			}

			// ������� ������ ������ �����
			body.clear();

			// ������� ������ ������ ��������
			substrings.clear();

			// ��������� ������ �� �����
			for (std::string line(""); std::getline(file, line); )
			{
				// ��������, ��� ������� ������ �� �����
				if (line.empty() == false)
				{
					body.push_back(line);
				}
			}

			// ��������� ����
			file.close();

			// ������� ����� ����������� �����
			const std::size_t lines = body.size();

			// ��������, ��� ���� ��� ����
			if (lines == 0)
			{
				// ������� � �������� ���� ���� �� �������� �����
				{
					// ��������� �������� ����
					std::unique_lock<std::mutex> lock(m_output_mtx);

					// ���������� � ���� ���� �� �������� �����
					*m_output << '[' << cur_path << ']' << std::endl << std::endl;
				}

				// ��������� � ���������� �����
				continue;
			}

			// ��������, ��� � ����� ����� 1 ������
			if (lines == 1)
			{
				// ������� � �������� ���� ���� �� �������� ����� � ������ ��� �������
				{
					// ��������� �������� ����
					std::unique_lock<std::mutex> lock(m_output_mtx);

					// ���������� � ���� ���� �� �������� �����
					*m_output << '[' << cur_path << ']' << std::endl << *body.cbegin() << std::endl << std::endl;
				}

				// ��������� � ���������� �����
				continue;
			}

			// ��������� � ������ �������� ������ ������
			substrings.push_back(*body.cbegin());

			// ��������� ���������
			for (auto it = ( body.cbegin() + 1) ; it != body.cend(); ++it)
			{
				// ����� ������� �����������
				const char d = (*it).front();

				// ��������� ������ ��� ��������
				std::vector<std::string> tmp;

				// ��������� ������ �� ���������
				for (const auto& str : substrings)
				{
					// ���������...
					utils::string::explode(str, tmp, d);
				}

				// ��������� ������ ��������
				substrings = std::move(tmp);
			}
		
			// ������� ��������� � �������� ����
			{
				// ��������� �������� ����
				std::unique_lock<std::mutex> lock(m_output_mtx);

				// ������� ���� �� �����
				*m_output << '[' << cur_path << ']' << std::endl;

				// ������� ���������
				for (const auto& str : substrings)
				{
					*m_output << str << std::endl;
				}

				// ������� ������� �� ����� ������
				*m_output << std::endl;
			}
		}
	}
}