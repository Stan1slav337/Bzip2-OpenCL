#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// Function to perform Run-Length Encoding on the input file
void runLengthEncode(const string &inputFile, const string &outputFile)
{
    return;
    ifstream fin(inputFile, ios::binary);
    if (!fin.is_open())
    {
        cerr << "Error: Unable to open input file." << endl;
        return;
    }

    ofstream fout(outputFile + ".rle", ios::binary);
    if (!fout.is_open())
    {
        cerr << "Error: Unable to create output file." << endl;
        fin.close();
        return;
    }

    char current, next;
    int count = 1;
    while (fin.get(current))
    {
        if (fin.peek() == EOF || current != fin.peek())
        {
            fout.put(count);
            fout.put(current);
            count = 1;
        }
        else
        {
            count++;
        }
    }

    fin.close();
    fout.close();
}

// Function to perform Run-Length Decoding on the input file
void runLengthDecode(const string &inputFile, const string &outputFile)
{
    return;
    ifstream fin(inputFile, ios::binary);
    if (!fin.is_open())
    {
        cerr << "Error: Unable to open input file." << endl;
        return;
    }

    ofstream fout(outputFile, ios::binary);
    if (!fout.is_open())
    {
        cerr << "Error: Unable to create output file." << endl;
        fin.close();
        return;
    }

    char count, character;
    while (fin.get(count) && fin.get(character))
    {
        for (int i = 0; i < count; ++i)
        {
            fout.put(character);
        }
    }

    fin.close();
    fout.close();
}