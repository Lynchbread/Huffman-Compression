#pragma once

#include <fstream>
#include <string>

#include "HuffmanTree.h"

class RL23
{
	// Compression functions
	static char* binary_str_to_compressed_str(char*, unsigned long long&);

	// Decompression functions
	static char* compressed_str_to_bin_str(const char*, unsigned long long);
	static char* bin_str_to_original_str(char*, unsigned long long, const HuffmanTree*);

public:
	RL23();
	~RL23();

	static std::string compress(const std::string&, const std::string&);
	static std::string decompress(const std::string&, const std::string&);
};