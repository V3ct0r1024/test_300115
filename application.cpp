#include "application.h"
#include "utils.h"

#include <vector>
#include <thread>
#include <functional>

#include <Windows.h>


namespace v3ct0r
{
	// Конструктор
	application::application() :
		m_output(nullptr)
	{

	}

	// Запустить программу
	application::return_errno_t application::run(int argc, char** argv)
	{
		// Проверим число параметров
		if (argc != 3) {
			return return_errno_t::wrong_params;
		}

		// Отформатируем путь до директории c файлами
		const std::string input_path = utils::string::format_path(argv[1]);

		// Получим путь до выходного файла
		const std::string output_path = std::string(argv[2]);

		// Удалим старый список файлов
		file_list_t().swap(m_files);

		// Ищем файлы в указанной директории
		search_files(input_path, m_files);

		// Проверим, что файлы не найдены
		if (m_files.empty() == true)
		{
			// Выходим из программы
			return return_errno_t::no_files_found;
		}

		// Открываем выходной файл на запись
		m_output = std::make_shared<std::ofstream>(output_path, std::ofstream::out | std::ofstream::trunc);

		// Проверим, что файл открыт
		if (m_output->is_open() == false)
		{
			// Не удалось открыть/создать выходной файл
			return return_errno_t::output_error;
		}

		// Получим число процессоров в системе
		const unsigned int processor_count = max(std::thread::hardware_concurrency(), 1);

		// Список рабочих потоков
		std::vector<std::thread> workers;
		
		// Запускаем processor_count - 1 рабочих потоков
		for (unsigned int i = 0; i < (processor_count - 1); ++i)
		{
			// Создаем поток . . .
			std::thread th( std::bind( std::mem_fn(&application::work), this ) );

			// . . . и добавляем его в список
			workers.push_back( std::move(th) );
		}

		// Обрабатываем файлы также основным потоком
		work();

		// Ждем когда рабочие потоки закончат выполнение
		std::for_each(workers.begin(), workers.end(), [](std::thread& t)
		{
			t.join();
		});

		// Закрываем выходной файл
		m_output->close();

		// Файлы обработаны
		return return_errno_t::success;
	}

	// Найти текстовые файлы в указанной директории
	void application::search_files(const std::string& path, file_list_t& files)
	{
		// Описывает файл, найденный функциями FindFirstFile, FindFirstFileEx, или FindNextFile
		WIN32_FIND_DATAA FindFileData;

		/* Открывает дескриптор поиска и возвращает информацию о первом файле,
				с именем, которое отвечает заданным условиям */
		HANDLE hFind = FindFirstFileA((path + "*.*").c_str(), &FindFileData);

		// Проверим, что найден хотя бы один файл
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				// Получим имя текущего файла
				const std::string file_name(FindFileData.cFileName);

				// Проверим атрибут файла - директория
				if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (file_name != "." && file_name != "..")
					{
						// Рекурсивно ищем файлы в субдиректории
						search_files((path + file_name + "\\"), files);
					}
				}
				else
				{
					// Добавляем очередной текстовый файл в список
					if (utils::string::extract_file_ext(file_name) == "txt")
					{
						files.push(path + file_name);
					}
				}
			} while (FindNextFileA(hFind, &FindFileData) != FALSE);
		}

		// Закрываем дескриптор поиска
		FindClose(hFind);
	}

	// Основная процедура рабочего потока
	void application::work()
	{
		// Взять путь до очередного файла из очереди
		auto take_path = [&mtx = m_files_mtx, &files = m_files](std::string& out) -> bool
		{
			// Блокируем список файлов
			std::unique_lock<std::mutex> lock(mtx);

			// Проверим, что в списке есть файлы
			if (files.empty() == true)
			{
				// В списке не осталось файлов
				return false;
			}

			// Берем файл
			const std::string path = files.front();

			// Удаляем файл из списка
			files.pop();

			// Передаем путь до файла по ссылке
			out = path;
	
			return true;
		};

		// Список строк из текущего файла
		std::vector<std::string> body;
		body.reserve(16);

		// Список подстрок
		std::vector<std::string> substrings;
		substrings.reserve(256);

		// Обрабатываем файлы . . .
		for (std::string cur_path(""); take_path(cur_path) == true; )
		{
			// Открываем очередной файл
			std::ifstream file(cur_path, std::ifstream::in);

			// Проверим, что файл открыт
			if (file.is_open() == false)
			{
				// Пропускаем файл
				continue;
			}

			// Удаляем старый список строк
			body.clear();

			// Удаляем старый список подстрок
			substrings.clear();

			// Считываем строки из файла
			for (std::string line(""); std::getline(file, line); )
			{
				// Проверим, что текущая строка не пуста
				if (line.empty() == false)
				{
					body.push_back(line);
				}
			}

			// Закрываем файл
			file.close();

			// Получим число прочитанных строк
			const std::size_t lines = body.size();

			// Проверим, что файл был пуст
			if (lines == 0)
			{
				// Запишем в выходной файл путь до текущего файла
				{
					// Блокируем выходной файл
					std::unique_lock<std::mutex> lock(m_output_mtx);

					// Записываем в него путь до текущего файла
					*m_output << '[' << cur_path << ']' << std::endl << std::endl;
				}

				// Переходим к следующему файлу
				continue;
			}

			// Проверим, что в файле всего 1 строка
			if (lines == 1)
			{
				// Запишем в выходной файл путь до текущего файла и строку для разбора
				{
					// Блокируем выходной файл
					std::unique_lock<std::mutex> lock(m_output_mtx);

					// Записываем в него путь до текущего файла
					*m_output << '[' << cur_path << ']' << std::endl << *body.cbegin() << std::endl << std::endl;
				}

				// Переходим к следующему файлу
				continue;
			}

			// Добавляем в список подстрок первую строку
			substrings.push_back(*body.cbegin());

			// Разбиваем подстроки
			for (auto it = ( body.cbegin() + 1) ; it != body.cend(); ++it)
			{
				// Берем текущий разделитель
				const char d = (*it).front();

				// Временный список для подстрок
				std::vector<std::string> tmp;

				// Разбиваем строку на подстроки
				for (const auto& str : substrings)
				{
					// Разбиваем...
					utils::string::explode(str, tmp, d);
				}

				// Обновляем список подстрок
				substrings = std::move(tmp);
			}
		
			// Запишем результат в выходной файл
			{
				// Блокируем выходной файл
				std::unique_lock<std::mutex> lock(m_output_mtx);

				// Запишем путь до файла
				*m_output << '[' << cur_path << ']' << std::endl;

				// Запишем подстроки
				for (const auto& str : substrings)
				{
					*m_output << str << std::endl;
				}

				// Запишем переход на новую строку
				*m_output << std::endl;
			}
		}
	}
}