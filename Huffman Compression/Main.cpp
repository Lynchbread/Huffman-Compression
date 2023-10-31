#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "RL23.h"

unsigned long long compare_files(const std::string&, const std::string&);

int main()
{
	const std::string uncompressed_filename = "uint_data1.dat";
	const std::string compressed_filename = "compressed_" + uncompressed_filename;
	const std::string decompressed_filename = "decompressed_" + uncompressed_filename;

	auto start_time = std::chrono::high_resolution_clock::now();
	std::cout << RL23::compress(uncompressed_filename, compressed_filename);
	auto stop_time = std::chrono::high_resolution_clock::now();
	std::cout << "Time Taken: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count()
		<< " ms\n";

	start_time = std::chrono::high_resolution_clock::now();
	std::cout << RL23::decompress(compressed_filename, decompressed_filename);
	stop_time = std::chrono::high_resolution_clock::now();
	std::cout << "Time Taken: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count()
		<< " ms\n";

	const auto line = compare_files(uncompressed_filename, decompressed_filename);

	if (line == 0)
		std::cout << "\nMatch!\n";
	else
		std::cout << "\nNot a match. Line: " << line;

	return 0;
}

unsigned long long compare_files(const std::string& filename1, const std::string& filename2)
{
	long long line_count = 0;

	std::ifstream infile1(filename1);

	if (!infile1.is_open())
		return line_count + 1;

	std::ifstream infile2(filename2);

	if (!infile2.is_open())
		return line_count + 1;

	while (!infile1.eof() && !infile2.eof())
	{
		line_count++;

		std::string line1;
		std::string line2;

		std::getline(infile1, line1);
		std::getline(infile2, line2);

		if (line1 != line2)
			return line_count;
	}

	infile1.close();
	infile2.close();

	return 0;
}