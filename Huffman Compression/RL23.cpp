#pragma warning(disable : 4996)

#include "RL23.h"

#include <bitset>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

char* RL23::binary_str_to_compressed_str(char* bin_str)
{
	const unsigned long long size = std::strlen(bin_str) / 8;
	const auto new_str = new char[size + 1];
	new_str[size] = '\0';

	unsigned long long pos = 0;

	for (; pos < size; pos++)
	{
		char temp_str[9];
		std::strncpy(temp_str, bin_str + pos * 8, 8);
		temp_str[8] = '\0';

		std::bitset<8> bitset(temp_str);
		const char c = static_cast<char>(bitset.to_ulong());
		new_str[pos] = c;
	}

	std::strcpy(bin_str, bin_str + pos * 8);

	return new_str;
}

char* RL23::compressed_str_to_bin_str(const char* comp_str, const unsigned long long old_size)
{
	const unsigned long long new_size = old_size * 8;
	const auto new_str = new char[new_size + 1];
	new_str[new_size] = '\0';

	for (unsigned long long i = 0; i < old_size; i++)
	{
		std::bitset<8> bitset(comp_str[i]);
		std::strcpy(new_str + i * 8, bitset.to_string().c_str());
	}

	return new_str;
}

char* RL23::bin_str_to_original_str(char* bin_str, HuffmanTree* tree)
{
	const unsigned long long old_size = std::strlen(bin_str);
	const auto new_str = new char[old_size];

	char data = 0;

	for (unsigned long long i = 0; i < old_size - 1 && data != -1; i++)
	{
		data = tree->GetData(bin_str);

		if (data != -1)
		{
			new_str[i] = data;
			new_str[i + 1] = '\0';
		}
	}

	return new_str;
}

RL23::RL23() {}

RL23::~RL23() {}

std::string RL23::compress(const std::string& input_filename, const std::string& output_filename)
{
	HuffmanTree tree(input_filename);

	std::cout << "Beginning compression...\n";

	std::ifstream infile(input_filename, std::ios::binary);

	if (!infile.is_open())
		return "ERROR: Failed to open file '" + input_filename + "'.\n";

	std::ofstream outfile(output_filename, std::ios::binary);

	outfile << tree << '\n';

	constexpr unsigned long long read_buffer_size = 4096;
	constexpr unsigned long long output_buffer_size = 65536;
	const auto read_buffer = new char[read_buffer_size];
	auto output_buffer = new char[output_buffer_size];
	output_buffer[0] = '\0';

	while (!infile.eof())
	{
		infile.read(read_buffer, read_buffer_size - 1);
		const unsigned long long gcount_size = infile.gcount();
		read_buffer[gcount_size] = '\0';

		for (unsigned long i = 0; i < gcount_size; i++)
		{
			auto temp_str = tree.GetCode(read_buffer[i]);

			if (std::strlen(output_buffer) + temp_str.length() + 1 > output_buffer_size)
			{
				unsigned long long output_str_len = std::strlen(output_buffer) / 8;
				char* output_str = binary_str_to_compressed_str(output_buffer);
				outfile.write(output_str, output_str_len);
				delete[] output_str;
			}

			std::strcat(output_buffer, temp_str.c_str());
		}
	}

	delete[] read_buffer;

	unsigned long long output_str_len = std::strlen(output_buffer) / 8;
	char* output_str = binary_str_to_compressed_str(output_buffer);
	outfile.write(output_str, output_str_len);
	delete[] output_str;

	outfile << '\n' << output_buffer;

	delete[] output_buffer;

	infile.close();
	outfile.close();

	return "Successfully compressed file '" + input_filename + "'.\n";
}

std::string RL23::decompress(const std::string& input_filename, const std::string& output_filename)
{
	std::cout << "Beginning decompression...\n";

	std::ifstream infile(input_filename, std::ios::binary);

	if (!infile.is_open())
		return "ERROR: Failed to open file '" + input_filename + "'.\n";

	HuffmanTree tree;

	infile >> tree;

	std::ofstream outfile(output_filename, std::ios::binary);

	constexpr unsigned long long read_buffer_size = 4096;
	constexpr unsigned long long output_buffer_size = read_buffer_size * 16;

	const auto read_buffer = new char[read_buffer_size];
	const auto binary_buffer = new char[read_buffer_size * 8 + 9];
	const auto output_buffer = new char[output_buffer_size];

	char* final_str = nullptr;

	binary_buffer[0] = '\0';
	output_buffer[0] = '\0';

	while (!infile.eof())
	{
		infile.read(read_buffer, read_buffer_size - 1);
		unsigned long long gcount_size = infile.gcount();
		read_buffer[gcount_size] = '\0';

		if (infile.eof())
		{
			final_str = read_buffer + gcount_size;

			for (; final_str[0] != '\n'; --final_str, --gcount_size);

			final_str[0] = '\0';
			final_str++;
		}

		const auto bin_str = compressed_str_to_bin_str(read_buffer, gcount_size);
		std::strcat(binary_buffer, bin_str);
		delete[] bin_str;

		if (infile.eof() && final_str != nullptr)
			std::strcat(binary_buffer, final_str);

		const char* original_str = bin_str_to_original_str(binary_buffer, &tree);

		if (std::strlen(output_buffer) + std::strlen(original_str) + 1 > output_buffer_size)
		{
			outfile.write(output_buffer, std::strlen(output_buffer));
			output_buffer[0] = '\0';
		}
			
		std::strcat(output_buffer, original_str);

		delete[] original_str;
	}

	outfile.write(output_buffer, std::strlen(output_buffer));

	delete[] read_buffer;
	delete[] binary_buffer;
	delete[] output_buffer;

	infile.close();
	outfile.close();

	return "Successfully decompressed file '" + input_filename + "'.\n";
}
