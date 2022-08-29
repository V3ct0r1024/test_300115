#include <iostream>
#include "application.h"
#include "utils.h"

// Точка входа
int main(int argc, char **argv)
{
	// Создаем экземпляр класса программы
	v3ct0r::application app;

	// Запускаем программу на выполнение
	const auto ret = app.run(argc, argv);

	// Выводим результат на консоль
	switch (ret)
	{
		case v3ct0r::application::return_errno_t::success:
			std::cout << "OK!" << std::endl;
			break;

		case v3ct0r::application::return_errno_t::no_files_found:
			std::cout << "No files found to process!" << std::endl;
			break;

		case v3ct0r::application::return_errno_t::output_error:
			std::cout << "Can't open (create) the output file!" << std::endl;
			break;

		case v3ct0r::application::return_errno_t::wrong_params:
			std::cout << "Wrong params! Usage: program.exe <path to files>" << std::endl;
			break;

		default:
			std::cout << "Unknown error!" << std::endl;
	}

	return 0;
}
