#include <iostream>
#include <fstream>
#include <vector>

#include "include/InputStream.hpp" // Assuming this is the header where BZip2InputStream is declared

int main(int argc, char *argv[])
{
    std::ifstream inputFile("output.bz2", std::ios::binary);        // The compressed input file
    std::ofstream outputFile("decompressed.txt", std::ios::binary); // The decompressed output file

    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open input file." << std::endl;
        return 1;
    }
    if (!outputFile.is_open())
    {
        std::cerr << "Failed to open output file." << std::endl;
        return 1;
    }

    InputStream bz2in(inputFile);

    int ch;
    // Read chunks from the decompressed stream
    while ((ch = bz2in.read()) != -1)
    {
        char x = ch;
        outputFile.write(&x, 1);
    }

    bz2in.close();

    return 0;
}
