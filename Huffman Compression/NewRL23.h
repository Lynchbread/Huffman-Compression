#pragma once

#include <fstream>
#include <string>

#include "NewHuffmanTree.h"

class NewRL23
{
	static char* binary_str_to_compressed_str(char*, unsigned long long&);
	static char* compressed_str_to_bin_str(const char*, unsigned long long);
	static char* bin_str_to_original_str(char*, unsigned long long, const NewHuffmanTree*);

public:
	NewRL23();
	~NewRL23();

	static std::string compress(const std::string&, const std::string&);
	static std::string decompress(const std::string&, const std::string&);
};