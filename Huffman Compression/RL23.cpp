#pragma warning(disable : 4996)

#include "RL23.h"

#include <cstring>
#include <iostream>

char* RL23::binary_str_to_compressed_str(char* bin_str, unsigned long long& bin_size)
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

char* RL23::compressed_str_to_bin_str(const char* comp_str, const unsigned long long old_size, char** code_arr)
{
	const unsigned long long new_size = old_size * 8;
	const auto new_str = new char[new_size + 1];
	new_str[new_size] = '\0';

	for (unsigned long long i = 0; i < old_size; i++)
	{
		const unsigned char c = comp_str[i];
		std::strcpy(new_str + i * 8, code_arr[c]);
	}

	return new_str;
}

char* RL23::bin_str_to_original_str(char* bin_str, const unsigned long long bin_length, const HuffmanTree* tree)
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

std::string RL23::check_conflicts(const std::string& original_filename)
{
	std::ifstream file(original_filename, std::ios::binary);

	const auto pos = original_filename.find_last_of('.');
	std::string new_filename = original_filename;

	for (int i = 1; file.is_open(); i++)
	{
		file.close();
		new_filename = original_filename;

		if (pos == std::string::npos)
			new_filename.append(" (" + std::to_string(i) + ")");
		else
			new_filename.insert(pos, " (" + std::to_string(i) + ")");

		file.open(new_filename, std::ios::binary);
	}

	return new_filename;
}

std::string RL23::compress(const std::string& input_filename, std::string output_filename)
{
	std::ifstream infile(input_filename, std::ios::binary);

	if (!infile.is_open())
		return "ERROR: Failed to open file '" + input_filename + "'.\n";

	if (output_filename.empty())
		output_filename = input_filename.substr(0, input_filename.find_last_of('.')) + ".r23";

	output_filename = check_conflicts(output_filename);

	HuffmanTree tree(input_filename);

	std::cout << "Beginning compression...\n";

	std::ofstream outfile(output_filename, std::ios::binary);

	outfile << input_filename << '\n' << tree << '\n';

	constexpr unsigned long long read_buffer_size = 4096;
	constexpr unsigned long long output_buffer_size = 65536;
	const auto read_buffer = new char[read_buffer_size];
	const auto output_buffer = new char[output_buffer_size];
	output_buffer[0] = '\0';
	
	while (!infile.eof())
	{
		infile.read(read_buffer, read_buffer_size - 1);
		const unsigned long long gcount_size = infile.gcount();
		read_buffer[gcount_size] = '\0';

		unsigned long long output_str_len = std::strlen(output_buffer);

		for (unsigned long long i = 0; i < gcount_size; i++)
		{
			const auto temp_str = tree.GetCode(read_buffer[i]);
			
			if (output_str_len + temp_str->length() + 1 > output_buffer_size)
			{
				const unsigned long long old_output_len = output_str_len / 8;
				const char* output_str = binary_str_to_compressed_str(output_buffer, output_str_len);
				outfile.write(output_str, old_output_len);
				delete[] output_str;
			}
			
			std::strcpy(output_buffer + output_str_len, temp_str->c_str());
			output_str_len += temp_str->length();
		}
	}

	delete[] read_buffer;

	unsigned long long output_str_len = std::strlen(output_buffer);
	const unsigned long long old_output_len = output_str_len / 8;
	const char* output_str = binary_str_to_compressed_str(output_buffer, output_str_len);
	outfile.write(output_str, static_cast<long long>(old_output_len));
	delete[] output_str;

	outfile << '\n' << output_buffer;

	delete[] output_buffer;

	infile.close();
	outfile.close();
	
	return "Successfully compressed file '" + input_filename + "'.\n";
}

std::string RL23::decompress(const std::string& input_filename)
{
	if (input_filename.substr(input_filename.length() - 4, std::string::npos) != ".r23")
		return "ERROR: '" + input_filename + "' is not a '.r23' file.\n";

	std::ifstream infile(input_filename, std::ios::binary);

	if (!infile.is_open())
		return "ERROR: Failed to open file '" + input_filename + "'.\n";

	std::string output_filename;
	std::getline(infile, output_filename);

	output_filename = check_conflicts(output_filename);

	if (output_filename.empty())
		return "ERROR: '" + input_filename + "' is missing its file list.\n";

	std::cout << "Beginning decompression...\n";

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

	const auto code_arr = new char* [256];

	for (int i = 0; i < 256; i++)
	{
		code_arr[i] = new char[9];
		code_arr[i][8] = '\0';

		int c = i;

		for (int j = 7; j >= 0; --j)
		{
			if (c % 2)
				code_arr[i][j] = '1';
			else
				code_arr[i][j] = '0';
			c /= 2;
		}
	}

	while (!infile.eof())
	{
		infile.read(read_buffer, read_buffer_size - 1);
		unsigned long long gcount_size = infile.gcount();
		read_buffer[gcount_size] = '\0';

		if (infile.eof())
		{
			final_str = read_buffer + gcount_size;

			for (; final_str[0] != '\n'; --final_str, --gcount_size) {}

			final_str[0] = '\0';
			final_str++;
		}

		const unsigned long long binary_leftover_length = std::strlen(binary_buffer);

		const auto bin_str = compressed_str_to_bin_str(read_buffer, gcount_size, code_arr);
		
		std::strcat(binary_buffer, bin_str);
		delete[] bin_str;

		unsigned long long binary_length = gcount_size * 8 + binary_leftover_length;

		if (infile.eof() && final_str != nullptr)
		{
			binary_length += std::strlen(final_str);
			std::strcat(binary_buffer, final_str);
		}

		const char* original_str = bin_str_to_original_str(binary_buffer, binary_length, &tree);
		
		if (std::strlen(output_buffer) + std::strlen(original_str) + 1 > output_buffer_size)
		{
			outfile.write(output_buffer, static_cast<long long>(std::strlen(output_buffer)));
			output_buffer[0] = '\0';
		}

		std::strcat(output_buffer, original_str);

		delete[] original_str;
	}

	for (int i = 0; i < 256; i++)
		delete[] code_arr[i];
	delete[] code_arr;

	outfile.write(output_buffer, static_cast<long long>(std::strlen(output_buffer)));

	delete[] read_buffer;
	delete[] binary_buffer;
	delete[] output_buffer;

	infile.close();
	outfile.close();

	return "Successfully decompressed file '" + input_filename + "'.\n";
}