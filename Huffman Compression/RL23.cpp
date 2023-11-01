#pragma warning(disable : 4996)

#include "RL23.h"

#include <bitset>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

unsigned long long RL23::bin_str_to_original_str_duration_ = 0;
unsigned long long RL23::compressed_str_to_bin_str_duration_ = 0;

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
	const auto start_time = std::chrono::high_resolution_clock::now();

	const unsigned long long new_size = old_size * 8;
	const auto new_str = new char[new_size + 1];
	new_str[new_size] = '\0';

	auto bin = new char[9];
	bin[8] = '\0';

	for (unsigned long long i = 0; i < old_size; i++)
	{
		//std::bitset<8> bitset(comp_str[i]);
		unsigned char c = comp_str[i];

		for (int j = 7; j >= 0; --j)
		{
			if (c % 2)
				bin[j] = '1';
			else
				bin[j] = '0';
			c /= 2;
		}

		//std::strcpy(new_str + i * 8, bitset.to_string().c_str());
		std::strcpy(new_str + i * 8, bin);
	}

	delete[] bin;

	compressed_str_to_bin_str_duration_ += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count();

	return new_str;
}

char* RL23::bin_str_to_original_str(char* bin_str, const HuffmanTree* tree)
{
	const auto start_time = std::chrono::high_resolution_clock::now();

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

	bin_str_to_original_str_duration_ += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count();

	return new_str;
}

RL23::RL23() : infile_(nullptr), tree_(nullptr), read_buffer_size_(0), read_buffer_(nullptr), binary_buffer_(nullptr) {}

RL23::RL23(const std::string& input_filename) : infile_(new std::ifstream(input_filename)), tree_(new HuffmanTree),
	read_buffer_size_(4096), read_buffer_(new char[read_buffer_size_]), binary_buffer_(new char[read_buffer_size_ * 8 + 9])
{
	binary_buffer_[0] = '\0';

	if (infile_->is_open())
		*infile_ >> *tree_;
}

RL23::~RL23()
{
	delete tree_;
	delete[] read_buffer_;
	delete[] binary_buffer_;
	infile_->close();
	delete infile_;
}

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

	std::cout << "compressed_str_to_bin_str() = " << compressed_str_to_bin_str_duration_ << " us\n";
	std::cout << "bin_str_to_original_str() = " << bin_str_to_original_str_duration_ << " us\n";
	std::cout << "GetData() = " << HuffmanTree::GetData_duration_ << " us\n";

	return "Successfully decompressed file '" + input_filename + "'.\n";
}

const char* RL23::decompress_partial() const
{
	if (!infile_->is_open())
		return nullptr;

	char* final_str = nullptr;

	infile_->read(read_buffer_, read_buffer_size_ - 1);
	unsigned long long gcount_size = infile_->gcount();
	read_buffer_[gcount_size] = '\0';

	if (infile_->eof())
	{
		final_str = read_buffer_ + gcount_size;

		for (; final_str[0] != '\n'; --final_str, --gcount_size);

		final_str[0] = '\0';
		final_str++;
	}

	const auto bin_str = compressed_str_to_bin_str(read_buffer_, gcount_size);
	std::strcat(binary_buffer_, bin_str);
	delete[] bin_str;

	if (infile_->eof() && final_str != nullptr)
		std::strcat(binary_buffer_, final_str);

	return bin_str_to_original_str(binary_buffer_, tree_);
}
