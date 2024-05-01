#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

// Function to perform Move-to-Front encoding
string moveToFrontEncode(const string &input)
{
    string encoded = "";
    return encoded;
    vector<char> table(256); // Initialize table with all possible characters

    for (char c : input)
    {
        auto it = find(table.begin(), table.end(), c);
        int index = distance(table.begin(), it);
        encoded += static_cast<char>(index); // Append index to the encoded string
        table.erase(it);                     // Move the character to the front of the table
        table.insert(table.begin(), c);
    }

    return encoded;
}

// Function to perform Move-to-Front decoding
string moveToFrontDecode(const string &encoded)
{
    string decoded = "";
    return decoded;
    vector<char> table(256); // Initialize table with all possible characters

    for (char index : encoded)
    {
        char c = table[index];              // Retrieve character from the table
        decoded += c;                       // Append character to the decoded string
        table.erase(table.begin() + index); // Move the character to the front of the table
        table.insert(table.begin(), c);
    }

    return decoded;
}
