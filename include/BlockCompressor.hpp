/*
 * Copyright (c) 2011 Matthew Francis
 * Copyright (c) 2024 Stanislav Brega
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef BLOCK_COMPRESSOR_HPP
#define BLOCK_COMPRESSOR_HPP

#include <vector>
#include <array>
#include <cstdint>
#include <algorithm>

#include "Config.hpp"
#include "CRC32.hpp"

class BlockCompressor
{
private:
    CRC32 crc{};
    int blockLength = 0;
    int blockLengthLimit;
    bool *blockValuesPresent;
    unsigned char *block;

    int rleCurrentValue = -1;
    int rleLength = 0;

public:
    BlockCompressor(unsigned char *blockPtr, bool *valuesPresentPtr, int blockSize) : block(blockPtr), // plus one to allow for the BWT wraparound
                                                                                      blockValuesPresent(valuesPresentPtr),
                                                                                      blockLengthLimit(blockSize - 6)
    {
    }

    bool isEmpty()
    {
        return blockLength == 0 && rleLength == 0;
    }

    int getCRC() const
    {
        return crc.getCRC();
    }

    int getBlockLength() const
    {
        return blockLength;
    }

    bool write(int value)
    {
        if (blockLength > blockLengthLimit)
        {
            return false;
        }

        if (rleLength == 0)
        {
            rleCurrentValue = value;
            rleLength = 1;
        }
        else if (rleCurrentValue == value)
        {
            if (++rleLength > 254)
            {
                writeRun(rleCurrentValue, 255);
                rleLength = 0;
            }
        }
        else
        {
            writeRun(rleCurrentValue, rleLength);
            rleCurrentValue = value;
            rleLength = 1;
        }
        return true;
    }

    int write(const std::vector<char> &data, int offset, int length)
    {
        int written = 0;
        while (length-- > 0)
        {
            if (!write(data[offset++]))
            {
                break;
            }
            ++written;
        }
        return written;
    }

    void finishRLE()
    {
        if (rleLength > 0)
        {
            writeRun(rleCurrentValue & 0xff, rleLength);
        }
    }

    void reset()
    {
        crc.reset();
        blockLength = 0;
        rleCurrentValue = -1;
        rleLength = 0;

        for (int i = 0; i < 256; ++i)
        {
            blockValuesPresent[i] = false;
        }
    }

private:
    void writeRun(int value, int runLength)
    {
        blockValuesPresent[value] = true;
        crc.updateCRC(value, runLength);
        block[blockLength++] = static_cast<unsigned char>(value);
        if (runLength > 1)
        {
            block[blockLength++] = static_cast<unsigned char>(value);
            if (runLength > 2)
            {
                block[blockLength++] = static_cast<unsigned char>(value);
                if (runLength > 3)
                {
                    runLength -= 4;
                    blockValuesPresent[runLength] = true;
                    block[blockLength++] = static_cast<unsigned char>(value);
                    block[blockLength++] = static_cast<unsigned char>(runLength);
                }
            }
        }
    }
};
#endif