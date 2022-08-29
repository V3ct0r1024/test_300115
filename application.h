#pragma once
#include <fstream>
#include <string>
#include <queue>
#include <mutex>


namespace v3ct0r
{
	// Класс приложения
	class application final
	{
		public:

			// Коды ошибок выполнения программы
			enum class return_errno_t : unsigned int
			{
				success        = 0,  // Успешно
				wrong_params   = 1,  // Некорректные параметры
				no_files_found = 2,  // Файлы для обработки не найдены
				output_error   = 3,  // Ошибка открытия (создания) файла с выходными данными
			};

			// Конструктор
			application();

			// Деструктор
			~application() = default;

			// Конструктор копирования
			application(const application&) = delete;

			// Оператор присваивания копированием
			application& operator=(const application&) = delete;

			// Конструктор перемещения
			application(application&&) = delete;

			// Оператор присваивания перемещением
			application& operator=(application&&) = delete;

			// Запустить программу
			return_errno_t run(int argc, char** argv);

		private:

			// Объявим тип для списка файлов
			using file_list_t = std::queue<std::string>;

			// Объявим тип умного указателя для std::ofstream
			using ofstream_ptr = std::shared_ptr<std::ofstream>;


			// Найти текстовые файлы в указанной директории
			static void search_files(const std::string& path, file_list_t& files);

			// Основная процедура рабочего потока
			void work();


			// Список файлов в заданной директории
			file_list_t m_files;
			mutable std::mutex m_files_mtx;

			// Выходной файл
			ofstream_ptr m_output;
			mutable std::mutex m_output_mtx;
	};
}


