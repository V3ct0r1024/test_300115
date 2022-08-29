#pragma once
#include <fstream>
#include <string>
#include <queue>
#include <mutex>


namespace v3ct0r
{
	// ����� ����������
	class application final
	{
		public:

			// ���� ������ ���������� ���������
			enum class return_errno_t : unsigned int
			{
				success        = 0,  // �������
				wrong_params   = 1,  // ������������ ���������
				no_files_found = 2,  // ����� ��� ��������� �� �������
				output_error   = 3,  // ������ �������� (��������) ����� � ��������� �������
			};

			// �����������
			application();

			// ����������
			~application() = default;

			// ����������� �����������
			application(const application&) = delete;

			// �������� ������������ ������������
			application& operator=(const application&) = delete;

			// ����������� �����������
			application(application&&) = delete;

			// �������� ������������ ������������
			application& operator=(application&&) = delete;

			// ��������� ���������
			return_errno_t run(int argc, char** argv);

		private:

			// ������� ��� ��� ������ ������
			using file_list_t = std::queue<std::string>;

			// ������� ��� ������ ��������� ��� std::ofstream
			using ofstream_ptr = std::shared_ptr<std::ofstream>;


			// ����� ��������� ����� � ��������� ����������
			static void search_files(const std::string& path, file_list_t& files);

			// �������� ��������� �������� ������
			void work();


			// ������ ������ � �������� ����������
			file_list_t m_files;
			mutable std::mutex m_files_mtx;

			// �������� ����
			ofstream_ptr m_output;
			mutable std::mutex m_output_mtx;
	};
}


