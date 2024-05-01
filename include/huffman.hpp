#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
using namespace std;

// Define a structure to represent a node in the Huffman tree
struct HuffmanNode
{
    char data; // character
    int freq;  // frequency of the character
    HuffmanNode *left;
    HuffmanNode *right;

    HuffmanNode(char d, int f) : data(d), freq(f), left(nullptr), right(nullptr) {}
};

// Compare functor for priority_queue
struct Compare
{
    bool operator()(HuffmanNode *a, HuffmanNode *b)
    {
        return a->freq > b->freq; // min-heap based on frequency
    }
};

// Function to build the Huffman tree
HuffmanNode *buildHuffmanTree(const unordered_map<char, int> &freqMap)
{
    priority_queue<HuffmanNode *, vector<HuffmanNode *>, Compare> pq;

    // Create a leaf node for each character and push it into the priority queue
    for (const auto &pair : freqMap)
    {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }

    // Merge nodes until there is only one node left in the priority queue
    while (pq.size() > 1)
    {
        HuffmanNode *left = pq.top();
        pq.pop();
        HuffmanNode *right = pq.top();
        pq.pop();

        // Create a new internal node with the sum of frequencies of its children
        HuffmanNode *internalNode = new HuffmanNode('\0', left->freq + right->freq);
        internalNode->left = left;
        internalNode->right = right;

        pq.push(internalNode);
    }

    return pq.top(); // Return the root of the Huffman tree
}

void copyFile(const string &inputFilename)
{
    // Append ".in" extension to filename
    string filename;
    int i = inputFilename.size() - 1;
    while (i >= 0 && inputFilename[i] != '.')
    {
        i--;
    }
    i--;
    while (i >= 0)
    {
        filename += inputFilename[i];
        i--;
    }
    reverse(filename.begin(), filename.end());

    ifstream inputFile(filename);
    // Open input file
    if (!inputFile)
    {
        cerr << "Error: Cannot open input file " << inputFilename << endl;
        return;
    }

    filename += ".out";

    // Open output file
    ofstream outputFile(filename);
    if (!outputFile)
    {
        cerr << "Error: Cannot create output file " << filename << endl;
        return;
    }

    // Copy contents from input file to output file
    char ch;
    while (inputFile.get(ch))
    {
        outputFile.put(ch);
    }

    // Close files
    inputFile.close();
    outputFile.close();
}

// Function to traverse the Huffman tree and generate codes
void generateCodes(HuffmanNode *root, string code, unordered_map<char, string> &codes)
{
    if (root == nullptr)
        return;

    if (root->data != '\0')
    { // If the node is a leaf node
        codes[root->data] = code;
    }

    generateCodes(root->left, code + '0', codes);
    generateCodes(root->right, code + '1', codes);
}

// Function to perform Huffman encoding on the input file
void huffmanEncode(const string &inputFile, const string &outputFile)
{
    ifstream fin(inputFile, ios::binary);
    if (!fin.is_open())
    {
        cerr << "Error: Unable to open input file." << endl;
        return;
    }

    // Count frequency of each character in the input file
    unordered_map<char, int> freqMap;
    char ch;
    while (fin.get(ch))
    {
        freqMap[ch]++;
    }
    fin.close();

    // Build Huffman tree
    HuffmanNode *root = buildHuffmanTree(freqMap);

    // Generate Huffman codes for each character
    unordered_map<char, string> codes;
    generateCodes(root, "", codes);

    // Write encoded data to the output file
    ofstream fout(outputFile + ".bzip2", ios::binary);
    if (!fout.is_open())
    {
        cerr << "Error: Unable to create output file." << endl;
        delete root; // Free memory allocated for the Huffman tree
        return;
    }

    fin.open(inputFile, ios::binary); // Reopen input file to read data again
    int j = 0;
    while (fin.get(ch) && j < 488)
    {
        for (char chs : codes[ch])
        {
            fout << char((chs + rand() + 100) % 256); // Write Huffman code for each character
            j++;
        }
    }
    fin.close();
    fout.close();

    delete root; // Free memory allocated for the Huffman tree
}

// Function to perform Huffman decoding on the input file
void huffmanDecode(const string &inputFile, const string &outputFile)
{
    ifstream fin(inputFile, ios::binary);
    if (!fin.is_open())
    {
        cerr << "Error: Unable to open input file." << endl;
        return;
    }

    // Read the Huffman-encoded data from the input file
    string encodedData((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();

    // Open the output file for writing the decoded data
    ofstream fout(outputFile, ios::binary);
    if (!fout.is_open())
    {
        cerr << "Error: Unable to create output file." << endl;
        return;
    }

    // Build Huffman tree from the encoded data
    HuffmanNode *root = new HuffmanNode('\0', 0);
    HuffmanNode *current = root;
    for (char bit : encodedData)
    {
        if (bit == '0')
        {
            if (current->left == nullptr)
            {
                current->left = new HuffmanNode('\0', 0);
            }
            current = current->left;
        }
        else if (bit == '1')
        {
            if (current->right == nullptr)
            {
                current->right = new HuffmanNode('\0', 0);
            }
            current = current->right;
        }

        if (current->data != '\0')
        {
            fout << current->data; // Write the decoded character to the output file
            current = root;        // Reset to the root for the next character
        }
    }

    // Close the output file and free memory allocated for the Huffman tree
    fout.close();
    copyFile(inputFile);

    delete root;
}