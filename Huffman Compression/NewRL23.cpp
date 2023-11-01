#pragma warning(disable : 4996)

#include "NewRL23.h"

//#include <bitset>
//#include <chrono>
#include <cstring>
#include <fstream>
//#include <iomanip>
#include <iostream>
#include <string>

char* NewRL23::binary_str_to_compressed_str(char* bin_str, unsigned long long& bin_size)
{
	const unsigned long long size = bin_size / 8;
	const auto new_str = new char[size + 1];
	new_str[size] = '\0';
	unsigned long long pos = 0;

	for (unsigned long u = 0; pos < size; pos++, u = 0)
	{
		for (unsigned long long j = pos * 8 + 7, pow = 1; j < bin_size && j >= pos * 8; --j, pow *= 2)
			if (bin_str[j] == '1')
				u += pow;

		new_str[pos] = static_cast<char>(u);
	}

	std::strcpy(bin_str, bin_str + pos * 8);
	bin_size -= pos * 8;

	return new_str;
}

char* NewRL23::compressed_str_to_bin_str(const char* comp_str, const unsigned long long old_size)
{
	const unsigned long long new_size = old_size * 8;
	const auto new_str = new char[new_size + 1];
	new_str[new_size] = '\0';

	const auto bin = new char[9];
	bin[8] = '\0';

	for (unsigned long long i = 0; i < old_size; i++)
	{
		unsigned char c = comp_str[i];

		for (int j = 7; j >= 0; --j)
		{
			if (c % 2)
				bin[j] = '1';
			else
				bin[j] = '0';
			c /= 2;
		}

		std::strcpy(new_str + i * 8, bin);
	}

	delete[] bin;

	return new_str;
}

char* NewRL23::bin_str_to_original_str(char* bin_str, const unsigned long long bin_length, const NewHuffmanTree* tree)
{
	const auto new_str = new char[bin_length];

	unsigned long long pos = 0;
	char data = 0;

	for (unsigned long long i = 0; i < bin_length - 1 && data != -1; i++)
	{
		data = tree->GetData(bin_str, pos, bin_length);

		if (data != -1)
		{
			new_str[i] = data;
			new_str[i + 1] = '\0';
		}
	}

	std::strcpy(bin_str, bin_str + pos);

	return new_str;
}

NewRL23::NewRL23() = default;

NewRL23::~NewRL23() = default;

std::string NewRL23::compress(const std::string& input_filename, const std::string& output_filename)
{
	NewHuffmanTree tree(input_filename);

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

		unsigned long long output_str_len = std::strlen(output_buffer);

		for (unsigned long i = 0; i < gcount_size; i++)
		{
			auto temp_str = tree.GetCode(read_buffer[i]);

			if (output_str_len + temp_str.length() + 1 > output_buffer_size)
			{
				const unsigned long long old_output_len = output_str_len / 8;
				char* output_str = binary_str_to_compressed_str(output_buffer, output_str_len);
				outfile.write(output_str, old_output_len);
				delete[] output_str;
			}

			output_str_len += temp_str.length();
			std::strcat(output_buffer, temp_str.c_str());
		}
	}

	delete[] read_buffer;

	unsigned long long output_str_len = std::strlen(output_buffer);
	const unsigned long long old_output_len = output_str_len / 8;
	char* output_str = binary_str_to_compressed_str(output_buffer, output_str_len);
	outfile.write(output_str, old_output_len);
	delete[] output_str;

	outfile << '\n' << output_buffer;

	delete[] output_buffer;

	infile.close();
	outfile.close();

	return "Successfully compressed file '" + input_filename + "'.\n";
}

std::string NewRL23::decompress(const std::string& input_filename, const std::string& output_filename)
{
	std::cout << "Beginning decompression...\n";

	std::ifstream infile(input_filename, std::ios::binary);

	if (!infile.is_open())
		return "ERROR: Failed to open file '" + input_filename + "'.\n";

	NewHuffmanTree tree;

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

		const unsigned long long binary_leftover_length = std::strlen(binary_buffer);

		const auto bin_str = compressed_str_to_bin_str(read_buffer, gcount_size);
		std::strcat(binary_buffer, bin_str);
		delete[] bin_str;

		unsigned long long binary_length = gcount_size * 8 + binary_leftover_length;

		if (infile.eof() && final_str != nullptr)
		{
			binary_length += std::strlen(final_str);
			std::strcat(binary_buffer, final_str);
		}

		unsigned long long test = std::strlen(binary_buffer);

		const char* original_str = bin_str_to_original_str(binary_buffer, binary_length, &tree);

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