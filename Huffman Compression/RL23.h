#pragma once

#include <string>

#include "HuffmanTree.h"

class RL23
{
	static char* binary_str_to_compressed_str(char*);
	static char* compressed_str_to_bin_str(const char*, unsigned long long);

	char* bin_str_to_original_str(char*) const;

	HuffmanTree* tree_;

public:
	RL23();
	~RL23();

	std::string compress(const std::string&, const std::string&);
	std::string decompress(const std::string&, const std::string&);
};

