#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "RL23.h"

int main(const int argc, char *argv[])
{
	std::string input_filename;
	std::string output_filename;

	const std::string output_flag = "-o";

	switch(argc)
	{
	case 1:
		std::cout << "ERROR: Missing command arguments";
		return -1;
	case 2:
		input_filename = argv[1];
		break;
	case 4:
		for (int i = 1; i < argc; i++)
		{
			if (argv[i] == output_flag)
			{
				i++;
				if (i < argc)
					output_filename = argv[i];
				else
				{
					std::cout << "ERROR: Missing -o argument";
					return -1;
				}
			}
			else
				input_filename = argv[i];
		}
		break;
	default:
		std::cout << "ERROR: Invalid command arguments";
		return -1;
	}

	const auto start_time = std::chrono::high_resolution_clock::now();

	if (input_filename.substr(input_filename.find_last_of('.'), std::string::npos) == ".r23")
		std::cout << RL23::decompress(input_filename);
	else
		std::cout << RL23::compress(input_filename, output_filename);

	const auto stop_time = std::chrono::high_resolution_clock::now();

	std::cout << "Time Taken: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count()
		<< " ms\n\n";

	return 0;
}