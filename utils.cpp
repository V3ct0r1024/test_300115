#include "utils.h"
#include <algorithm>


namespace v3ct0r
{
	// Отформатировать строку с путем до файла
	std::string utils::string::format_path(const std::string& path)
	{
		// Копия исходной строки
		std::string copy(path);

		// Заменим слеши на обратные
		std::replace(copy.begin(), copy.end(), '/', '\\');

		// Добавим обратный слеш в конец
		if (copy.empty() == false && copy.back() != '\\') {
			copy.push_back('\\');
		}

		return copy;
	}

	// Извлечь расширение файла из пути файла
	std::string utils::string::extract_file_ext(const std::string& path)
	{
		const std::string::size_type pos = path.rfind('.');

		if (pos == std::string::npos) {
			return "";
		}

		return path.substr(pos + 1);
	}

	// Разбить строку по разделителю
	void utils::string::explode(const std::string& in, std::vector<std::string>& out, const char delim)
	{
		// Проверим, что входная строка не пуста
		if (in.empty() == true) {
			return;
		}

		// Текущая и начальная позиции поиска разделителя
		std::size_t pos = 0, start = 0;

		// Loop over the string
		while ( (pos = in.find(delim, pos)) != std::string::npos )
		{
			// Добавляем подстроку в список
			out.push_back( in.substr(start, pos - start) );

			// Устанавливаем новую начальную позицию поиска разделителя
			start = ++pos;
		} 
	
		// Добавляем в список последнюю подстроку
		out.push_back( in.substr(start, pos - start) );
	}
}