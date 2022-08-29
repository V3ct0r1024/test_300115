#pragma once
#include <string>
#include <vector>


namespace v3ct0r
{
	// Вспомогательные функции
	namespace utils
	{
		// Функции для работы со строками
		class string final
		{
			public:

				// Отформатировать строку с путем до файла
				static std::string format_path(const std::string& path);

				// Извлечь расширение файла из пути файла
				static std::string extract_file_ext(const std::string& path);

				// Разбить строку по разделителю
				static void explode(const std::string& in, std::vector<std::string>& out, const char delim);
		};
	};
}
