#include <iostream>

#include "include/huffman.hpp"
#include "include/bwt.hpp"
#include "include/mtf.hpp"
#include "include/rle.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        cerr << "Usage: " << argv[0] << " <input_file> [--dec]" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = inputFile;

    if (argc == 2)
    {
        runLengthEncode(inputFile, outputFile);
        burrowsWheelerEncode(outputFile);
        moveToFrontEncode(outputFile);
        runLengthEncode(outputFile, outputFile);
        huffmanEncode(inputFile, outputFile);
        cout << "Encoding completed." << endl;
    }
    else if (argc == 3 && string(argv[2]) == "--dec")
    {
        huffmanDecode(inputFile, outputFile);
        runLengthDecode(outputFile, outputFile);
        moveToFrontDecode(outputFile);
        burrowsWheelerDecode(outputFile);
        runLengthDecode(inputFile, outputFile);
        cout << "Decoding completed." << endl;
    }
    else
    {
        cerr << "Invalid arguments. Use '--dec' flag for decoding." << endl;
        return 1;
    }

    return 0;
}
