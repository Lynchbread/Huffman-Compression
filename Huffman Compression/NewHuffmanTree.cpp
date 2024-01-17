#pragma warning(disable : 4996)

#include "NewHuffmanTree.h"

#include <fstream>
#include <list>

NewHuffmanTree::Node::Node(const char c, const unsigned long long f)
{
	data = c;
	frequency = f;
	left = nullptr;
	right = nullptr;
}

NewHuffmanTree::Node::Node(Node* l, Node* r)
{
	data = -1;
	frequency = 0;
	left = l;
	right = r;
}

NewHuffmanTree::Node::Node(const Node& other)
{
	data = other.data;
	frequency = other.frequency;

	if (other.left != nullptr)
		left = new Node(*other.left);
	else
		left = nullptr;

	if (other.right != nullptr)
		right = new Node(*other.right);
	else
		right = nullptr;
}

NewHuffmanTree::Node* NewHuffmanTree::Node::get_node(const char bit) const
{
	if (bit == '1')
		return right;
	return left;
}

unsigned long long NewHuffmanTree::Node::get_frequency() const
{
	unsigned long long f = frequency;

	if (right)
		f += right->get_frequency();

	if (left)
		f += left->get_frequency();

	return f;
}

std::string NewHuffmanTree::traverse_tree(const Node* node)
{
	std::string str;

	if (node->left)
		str += traverse_tree(node->left);

	if (node->right)
		str += traverse_tree(node->right);

	if (node->data != -1)
		str.append(1, node->data);

	return str;
}

void NewHuffmanTree::GenerateCodeTable(const Node* current_node, const std::string& code)
{
	if (current_node == nullptr)
		return;

	if (current_node->data == -1)
	{
		GenerateCodeTable(current_node->left, code + '0');
		GenerateCodeTable(current_node->right, code + '1');
	}
	else
	{
		code_table_[current_node->data] = code;
	}
}

void NewHuffmanTree::LoadTree(const std::string& str)
{
	clear();

	head_ = new Node(nullptr, nullptr);

	for (unsigned long long strpos = 0;
		strpos != std::string::npos;
		strpos = str.find(',', strpos))
	{
		if (str[strpos] == ',')
			strpos++;

		const char data = str[strpos];

		std::string node_str = str.substr(strpos, str.find(',', strpos + 1) - strpos);

		node_str = node_str.substr(2);

		Node* temp_ptr = head_;

		for (const char c : node_str)
		{
			if (temp_ptr->get_node(c) == nullptr)
			{
				if (c == '0')
					temp_ptr->left = new Node(nullptr, nullptr);
				else
					temp_ptr->right = new Node(nullptr, nullptr);
			}

			temp_ptr = temp_ptr->get_node(c);
		}

		temp_ptr->data = data;
	}
}

NewHuffmanTree::NewHuffmanTree() : head_(nullptr) {}

NewHuffmanTree::NewHuffmanTree(const std::string& input_filename)
{
	std::ifstream infile(input_filename, std::ios::binary);

	if (infile.is_open())
	{
		constexpr int buffer_size = 4096;
		constexpr int frequency_arr_size = 128;
		const auto buffer = new char[buffer_size];
		unsigned long long frequency_arr[frequency_arr_size] = { 0 };

		while (!infile.eof())
		{
			infile.read(buffer, buffer_size);	// buffer does not have null-terminator

			for (unsigned long long i = infile.gcount(); i > 0; --i)
				frequency_arr[buffer[i]]++;
		}

		delete[] buffer;
		infile.close();

		std::list<Node> leaf_list;

		for (int i = 0; i < frequency_arr_size; i++)
			if (frequency_arr[i])
				leaf_list.emplace_back(i, frequency_arr[i]);

		leaf_list.sort();

		// Combine two smallest until only 1 remains. That 1 is head.
		while (leaf_list.size() > 1)
		{
			leaf_list.sort();

			const auto left = new Node(leaf_list.front());
			leaf_list.pop_front();
			const auto right = new Node(leaf_list.front());
			leaf_list.pop_front();

			leaf_list.emplace_front(left, right);
		}

		head_ = new Node(leaf_list.front());

		GenerateCodeTable(head_, "");
	}
}

void NewHuffmanTree::clear(const Node* node)
{
	if (head_ == nullptr)
		return;

	if (node == nullptr)
		node = head_;

	if (node->left)
		traverse_tree(node->left);

	if (node->right)
		traverse_tree(node->right);

	delete node;

	if (node == head_)
		head_ = nullptr;
}

char NewHuffmanTree::GetData(const char* bin_str, unsigned long long& pos, const unsigned long long size) const
{
	const Node* temp_ptr = head_;
	const unsigned long long old_pos = pos;

	for (; pos < size && temp_ptr->data == -1; pos++)
		temp_ptr = temp_ptr->get_node(bin_str[pos]);

	if (temp_ptr->data == -1)
		pos = old_pos;

	return temp_ptr->data;
}

std::string NewHuffmanTree::GetCode(const char c) { return code_table_[c]; }

bool operator<(const NewHuffmanTree::Node& l, const NewHuffmanTree::Node& r) { return l.get_frequency() < r.get_frequency(); }

std::istream& operator>>(std::istream& is, NewHuffmanTree& tree)
{
	std::string line;
	std::getline(is, line);

	if (line.back() == ',')
	{
		std::string line2;
		std::getline(is, line2);
		line += "\n" + line2;
	}

	tree.LoadTree(line);

	return is;
}

std::ostream& operator<<(std::ostream& os, const NewHuffmanTree& tree)
{
	const std::string str = tree.traverse_tree(tree.head_);
	std::string full_string;

	for (const char c : str)
	{
		full_string.append(1, c);
		full_string.append(1, '=');
		full_string.append(tree.code_table_[c]);

		if (c != str.back())
			full_string.append(1, ',');
	}

	os << full_string;

	return os;
}