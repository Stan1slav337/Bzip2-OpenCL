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

#ifndef OUTPUT_STREAM_HPP
#define OUTPUT_STREAM_HPP

#include <ostream>
#include <memory>
#include <bitset>

#include "BitOutputStream.hpp"
#include "BlockCompressor.hpp"

class OutputStream
{
private:
    std::ostream &outputStream;
    bool streamFinished = false;
    int streamBlockSize;
    int streamCRC = 0;
    int compressorIdx = 0;
    std::vector<BlockCompressor> blockCompressors{};

public:
    OutputStream(std::ostream &out, int blockSizeMultiplier, int parallelBlockCnt) : outputStream(out), streamBlockSize(blockSizeMultiplier * 100000)
    {
        if (blockSizeMultiplier < 1 || blockSizeMultiplier > 9)
        {
            throw std::invalid_argument("Invalid block size");
        }

        if (parallelBlockCnt < 1)
        {
            throw std::invalid_argument("Invalid parallel block count");
        }

        for (int i = 0; i < parallelBlockCnt; ++i)
        {
            blockCompressors.emplace_back(streamBlockSize);
        }
        // Initial block has stream start info
        auto &blockOutputStream = blockCompressors[0].getBitOutputStream();
        blockOutputStream.writeBits(16, STREAM_START_MARKER_1);
        blockOutputStream.writeBits(8, STREAM_START_MARKER_2);
        blockOutputStream.writeBits(8, '0' + blockSizeMultiplier);
    }

    void write(int value)
    {
        if (streamFinished)
        {
            throw std::runtime_error("Write beyond end of stream");
        }
        if (!blockCompressors[compressorIdx].write(value & 0xff))
        {
            getNextCompressor();
            blockCompressors[compressorIdx].write(value & 0xff);
        }
    }

    void write(const std::vector<char> &data, int offset, int length)
    {
        if (streamFinished)
        {
            throw std::runtime_error("Write beyond end of stream");
        }

        int bytesWritten = 0;
        while (length > 0)
        {
            if ((bytesWritten = blockCompressors[compressorIdx].write(data, offset, length)) < length)
            {
                getNextCompressor();
            }
            offset += bytesWritten;
            length -= bytesWritten;
        }
    }

    void close()
    {
        if (!streamFinished)
        {
            streamFinished = true;
            closeBlocks();
            auto &blockOutputStream = blockCompressors[0].getBitOutputStream();
            blockOutputStream.writeBits(24, STREAM_END_MARKER_1);
            blockOutputStream.writeBits(24, STREAM_END_MARKER_2);
            blockOutputStream.writeInteger(streamCRC);
            blockOutputStream.padding();
            blockOutputStream.writeFileBytes(outputStream, {}); // No leftover
            outputStream.flush();
        }
    }

private:
    void getNextCompressor()
    {
        compressorIdx++;
        // std::cout << compressorIdx << std::endl;

        if (compressorIdx == blockCompressors.size())
        {
            closeBlocks();
            compressorIdx = 0;
        }
    }

    void closeBlocks()
    {
        for (auto &blockCompressor : blockCompressors)
        {
            closeBlock(blockCompressor);
        }

        std::vector<bool> leftBuffer{};
        for (auto &blockCompressor : blockCompressors)
        {
            if (!blockCompressor.isEmpty())
            {
                int blockCRC = blockCompressor.getCRC();
                streamCRC = ((streamCRC << 1) | (static_cast<unsigned int>(streamCRC) >> 31)) ^ blockCRC;

                auto &blockOutputStream = blockCompressor.getBitOutputStream();
                blockOutputStream.writeFileBytes(outputStream, leftBuffer);
                leftBuffer = blockOutputStream.getLeftBuffer();
            }
            blockCompressor.reset();
        }

        // Leftover is written in the next iteration
        auto &blockOutputStream = blockCompressors[0].getBitOutputStream();
        blockOutputStream.writeFileBytes(outputStream, leftBuffer);
    }

    void closeBlock(BlockCompressor &blockCompressor)
    {
        if (!blockCompressor.isEmpty())
        {
            blockCompressor.close();
        }
    }
};
#endif