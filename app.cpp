#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

#include "include/OutputStream.hpp"
#include "include/InputStream.hpp"

int main(int argc, char *argv[])
{
    const char *flags = "\n\n  [--help|-h]              print help\n  [--dec|-d]               decompress file\n  [--keep|-k]              keep original (de)compressed file\n  [--check|-c]             check compressed file integrity\n  [--size|-s <1-9>]        set block size 10k .. 90k\n  [--parallel|-p <1+>]     number of parallel threads for gpu\n";
    if (argc < 2)
    {
        std::cerr << "\n  Usage: .\\bzip2.exe [file_path] [flags]" << flags << std::endl;
        return 1;
    }

    std::string filename;
    bool decompress = false;
    bool keepFile = false;
    bool checkCRC = false;
    int blockSize = 9;    // Default block size
    int parallelCnt = 10; // Default parallel blocks

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--dec") == 0 || std::strcmp(argv[i], "-d") == 0)
        {
            decompress = true;
        }
        else if (std::strcmp(argv[i], "--keep") == 0 || std::strcmp(argv[i], "-k") == 0)
        {
            keepFile = true;
        }
        else if ((std::strcmp(argv[i], "--size") == 0 || std::strcmp(argv[i], "-s") == 0) && i + 1 < argc)
        {
            blockSize = std::atoi(argv[++i]);
        }
        else if ((std::strcmp(argv[i], "--parallel") == 0 || std::strcmp(argv[i], "-p") == 0) && i + 1 < argc)
        {
            parallelCnt = std::atoi(argv[++i]);
        }
        else if (std::strcmp(argv[i], "--check") == 0 || std::strcmp(argv[i], "-c") == 0)
        {
            checkCRC = true;
        }
        else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0)
        {
            std::cout << "\n  Usage: .\\bzip2.exe [file_path] [flags]" << flags << std::endl;
            return 0;
        }
        else if (argv[i][0] == '-')
        {
            std::cerr << "  Unknown flag!\n\n  Usage: .\\bzip2.exe [file_path] [flags]" << flags << std::endl;
            return 1;
        }
        else
        {
            filename = argv[1];
        }
    }

    std::ifstream inputFile(filename, std::ios::binary);
    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open input file." << std::endl;
        return 1;
    }

    if (!decompress && !checkCRC)
    {
        std::string outputFilename = filename + ".bz2";
        std::ofstream outputFile(outputFilename, std::ios::binary);
        if (!outputFile.is_open())
        {
            std::cerr << "Failed to open output file.";
            return 1;
        }

        OutputStream bz2out(outputFile, blockSize, parallelCnt);

        const size_t bufferSize = 131072;
        std::vector<char> buffer(bufferSize);

        while (inputFile)
        {
            inputFile.read(buffer.data(), bufferSize);
            std::streamsize bytes_read = inputFile.gcount();
            for (int i = 0; i < bytes_read; ++i)
            {
                bz2out.write(buffer[i]);
            }
        }
        bz2out.close();

        if (!keepFile)
        {
            std::remove(filename.c_str());
        }

        outputFile.close();
    }
    else
    {
        if (!checkCRC)
        {
            if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".bz2")
            {
                std::cerr << "Input file doesn't have right .bz2 extenstion for decompression!" << std::endl;
                return 1;
            }

            std::string outputFilename = filename.substr(0, filename.size() - 4); // Remove ".bz2"
            if (!checkCRC && std::ifstream(outputFilename))
            {
                outputFilename += ".out";
                std::cerr << "Warning: Output file exists. Writing to " << outputFilename << std::endl;
            }
            std::ofstream outputFile(outputFilename, std::ios::binary);
            if (!outputFile.is_open())
            {
                std::cerr << "Failed to open output file." << std::endl;
                return 1;
            }

            InputStream bz2in(inputFile);

            int ch;
            while ((ch = bz2in.read()) != -1)
            {
                char x = ch;
                outputFile.write(&x, 1);
            }
            bz2in.close();

            outputFile.close();
        }
        else
        {
            InputStream bz2in(inputFile);
            while (bz2in.read() != -1)
                ;
            bz2in.close();
            std::cout << "  Integrity check passed!" << std::endl;
        }
    }

    inputFile.close();

    if (!keepFile && !checkCRC)
    {
        std::remove(filename.c_str());
    }

    return 0;
}
