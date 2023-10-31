#pragma once

#include <fstream>
#include <string>

#include "HuffmanTree.h"

class RL23
{
	static char* binary_str_to_compressed_str(char*);
	static char* compressed_str_to_bin_str(const char*, unsigned long long);
	static char* bin_str_to_original_str(char*, HuffmanTree*);

	std::ifstream* infile_;
	HuffmanTree* tree_;

	const unsigned long long read_buffer_size_;
	char* read_buffer_;
	char* binary_buffer_;

public:
	RL23();
	RL23(const std::string&);
	~RL23();

	static std::string compress(const std::string&, const std::string&);
	static std::string decompress(const std::string&, const std::string&);

	const char* decompress_partial() const;
};

