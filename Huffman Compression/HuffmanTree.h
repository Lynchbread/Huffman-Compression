#pragma once

#include <string>

class HuffmanTree
{
	struct Node
	{
		char data;
		unsigned long long frequency;

		Node* left;
		Node* right;

		Node(char, unsigned long long);
		Node(Node*, Node*);
		Node(const Node&);

		Node* get_node(char) const;

		unsigned long long get_frequency() const;
	};

	std::string code_table_[128];
	Node* head_;

	static std::string traverse_tree(const Node*);

	void GenerateCodeTable(const Node*, const std::string&);
	void LoadTree(const std::string&);

public:
	HuffmanTree();
	HuffmanTree(const std::string&);

	void clear(const Node* = nullptr);
	char GetData(const char*, unsigned long long&, unsigned long long) const;
	std::string GetCode(char);

	friend bool operator<(const Node&, const Node&);
	friend std::istream& operator>>(std::istream& is, HuffmanTree& tree);
	friend std::ostream& operator<<(std::ostream& os, const HuffmanTree& tree);
};