#include <iostream>
#include <fstream>

#include "include/BitInputStream.hpp"
#include "include/BitOutputStream.hpp"
#include "include/CRC32.hpp"

int main(int argc, char *argv[])
{
    std::filebuf fbIn;
    if (!fbIn.open("1.txt", std::ios::in))
    {
        throw std::runtime_error("Cannot open input file");
    }
    std::istream iStream{&fbIn};
    BitInputStream bStreamInput{iStream};

    std::filebuf fbOut;
    if (!fbOut.open("out.txt", std::ios::out))
    {
        throw std::runtime_error("Cannot open output file");
    }
    std::ostream oStream{&fbOut};
    BitOutputStream bStreamOutput{oStream};

    bStreamOutput.writeInteger(bStreamInput.readInteger());

    // for (int i = 0; i < 8 * 4; ++i)
    // {
    //     bStreamOutput.writeBoolean(bStreamInput.readBoolean());
    // }

    return 0;
}
