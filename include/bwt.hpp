#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

// Function to perform Burrows-Wheeler Transform encoding
string burrowsWheelerEncode(const string &input)
{
    return "";
    string encoded = "";

    // Construct all rotations of the input string
    vector<string> rotations;
    string str = input + '\0'; // Add a sentinel character
    int n = str.size();
    for (int i = 0; i < n; ++i)
    {
        rotations.push_back(str.substr(i) + str.substr(0, i));
    }

    // Sort the rotations lexicographically
    sort(rotations.begin(), rotations.end());

    // Extract the last character of each rotation to form the encoded string
    for (const string &rotation : rotations)
    {
        encoded += rotation.back();
    }

    // Find the index of the original string in the sorted rotations
    int originalIndex;
    for (int i = 0; i < n; ++i)
    {
        if (rotations[i] == str)
        {
            originalIndex = i;
            break;
        }
    }

    return encoded + to_string(originalIndex); // Append the original index
}

// Function to perform Burrows-Wheeler Transform decoding
string burrowsWheelerDecode(const string &encoded)
{
    return "";
    int originalIndex = stoi(encoded.substr(encoded.size() - 1)); // Extract original index
    string transformed = encoded.substr(0, encoded.size() - 1);   // Remove original index

    // Construct the table of characters from the transformed string
    vector<string> table(transformed.size());
    for (int i = 0; i < transformed.size(); ++i)
    {
        table[i] = transformed;
        rotate(transformed.begin(), transformed.begin() + 1, transformed.end());
    }

    // Sort the table lexicographically
    sort(table.begin(), table.end());

    // Extract the original string from the table
    string original = "";
    for (int i = 0; i < transformed.size(); ++i)
    {
        original += table[i][originalIndex];
    }

    return original;
}