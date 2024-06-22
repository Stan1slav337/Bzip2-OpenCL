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
#include "opencl.hpp"

class OutputStream
{
private:
    std::ostream &outputStream;
    bool streamFinished = false;
    int streamBlockSize;
    int parallelBlockCnt;
    int streamCRC = 0;
    int compressorIdx = 0;
    size_t BIT_BLOCK_MAX_SIZE;
    std::vector<BlockCompressor> blockCompressors{};
    Memory<bool> bitOutBuffers{};
    Memory<size_t> bitOutCnts{};
    Memory<unsigned char> inputBlocks{};
    Memory<size_t> inputBlockSizes{};
    Memory<int> bwtBlocks{};
    Memory<int> bwtBucketsA{};
    Memory<int> bwtBucketsB{};
    Memory<int> bwtTempBuffs{};
    Memory<bool> blocksValuePresent{};
    Memory<bool> isEmptyCompressor{};
    Memory<int> mtfsSymbolFrequencies{};
    Memory<int> huffmanSymbolMaps{};
    Memory<int> symbolMTFs{};
    Memory<int> huffmanSelectors{};
    // Device device;
    Device device{select_device_with_most_flops()};
    std::unique_ptr<Kernel> kernel_close;

public:
    OutputStream(std::ostream &out,
                 int blockSizeMultiplier,
                 int parallelBlockCnt) : outputStream(out),
                                         streamBlockSize(BLOCKSIZE_DEFAULT * blockSizeMultiplier),
                                         parallelBlockCnt(parallelBlockCnt),
                                         BIT_BLOCK_MAX_SIZE(16ll * streamBlockSize)

    {
        if (blockSizeMultiplier < 1 || blockSizeMultiplier > 9)
        {
            throw std::invalid_argument("Invalid block size");
        }

        if (parallelBlockCnt < 1)
        {
            throw std::invalid_argument("Invalid parallel block count");
        }

        bitOutBuffers = Memory<bool>(device, BIT_BLOCK_MAX_SIZE * parallelBlockCnt);
        bitOutCnts = Memory<size_t>(device, parallelBlockCnt);
        isEmptyCompressor = Memory<bool>(device, parallelBlockCnt);
        inputBlocks = Memory<unsigned char>(device, streamBlockSize * parallelBlockCnt);
        inputBlockSizes = Memory<size_t>(device, parallelBlockCnt);
        bwtBlocks = Memory<int>(device, streamBlockSize * parallelBlockCnt);
        bwtBucketsA = Memory<int>(device, BWT_BUCKET_A_SIZE * parallelBlockCnt);
        bwtBucketsB = Memory<int>(device, BWT_BUCKET_B_SIZE * parallelBlockCnt);
        bwtTempBuffs = Memory<int>(device, ALPHABET_SIZE * parallelBlockCnt);
        blocksValuePresent = Memory<bool>(device, ALPHABET_SIZE * parallelBlockCnt);
        mtfsSymbolFrequencies = Memory<int>(device, ALPHABET_SIZE * parallelBlockCnt);
        huffmanSymbolMaps = Memory<int>(device, ALPHABET_SIZE * parallelBlockCnt);
        symbolMTFs = Memory<int>(device, ALPHABET_SIZE * parallelBlockCnt);
        huffmanSelectors = Memory<int>(device, ((streamBlockSize + HUFFMAN_GROUP_RUN_LENGTH - 1) / HUFFMAN_GROUP_RUN_LENGTH) * parallelBlockCnt);

        kernel_close.reset(new Kernel{device,
                                      parallelBlockCnt,
                                      "kernel_close",
                                      isEmptyCompressor,
                                      inputBlocks,
                                      bwtBlocks,
                                      inputBlockSizes,
                                      bwtBucketsA,
                                      bwtBucketsB,
                                      bwtTempBuffs,
                                      bitOutBuffers,
                                      bitOutCnts,
                                      blocksValuePresent,
                                      mtfsSymbolFrequencies,
                                      huffmanSymbolMaps,
                                      symbolMTFs,
                                      huffmanSelectors,
                                      parallelBlockCnt,
                                      streamBlockSize});

        for (int i = 0; i < parallelBlockCnt; ++i)
        {
            blockCompressors.emplace_back(inputBlocks.data() + i * streamBlockSize,
                                          blocksValuePresent.data() + i * ALPHABET_SIZE,
                                          streamBlockSize);
        }

        // Initial block has stream start info
        writeBits(bitOutBuffers.data(), &(bitOutCnts[0]), 16, STREAM_START_MARKER_1);
        writeBits(bitOutBuffers.data(), &(bitOutCnts[0]), 8, STREAM_START_MARKER_2);
        writeBits(bitOutBuffers.data(), &(bitOutCnts[0]), 8, '0' + blockSizeMultiplier);
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
            writeBits(bitOutBuffers.data(), &(bitOutCnts[0]), 24, STREAM_END_MARKER_1);
            writeBits(bitOutBuffers.data(), &(bitOutCnts[0]), 24, STREAM_END_MARKER_2);
            writeInteger(bitOutBuffers.data(), &(bitOutCnts[0]), streamCRC);
            padding(bitOutBuffers.data(), &(bitOutCnts[0]));
            writeFileBytes(bitOutBuffers.data(), &(bitOutCnts[0]), outputStream, {}); // No leftover
            outputStream.flush();
        }
    }

private:
    void getNextCompressor()
    {
        compressorIdx++;

        if (compressorIdx == blockCompressors.size())
        {
            closeBlocks();
            compressorIdx = 0;
        }
    }

    void closeBlocks()
    {
        for (int i = 0; i < blockCompressors.size(); ++i)
        {
            auto &blockCompressor = blockCompressors[i];
            isEmptyCompressor[i] = blockCompressor.isEmpty();

            if (!isEmptyCompressor[i])
            {
                blockCompressor.finishRLE();

                int blockCRC = blockCompressor.getCRC();
                streamCRC = ((streamCRC << 1) | (static_cast<unsigned int>(streamCRC) >> 31)) ^ blockCRC;

                inputBlockSizes[i] = blockCompressor.getBlockLength();

                bool *bitBuffer = bitOutBuffers.data() + i * 16 * streamBlockSize;
                size_t *bitCount = &(bitOutCnts[i]);
                writeBits(bitBuffer, bitCount, 24, BLOCK_HEADER_MARKER_1);
                writeBits(bitBuffer, bitCount, 24, BLOCK_HEADER_MARKER_2);
                writeInteger(bitBuffer, bitCount, blockCRC);
                writeBoolean(bitBuffer, bitCount, false); // Randomised block flag
            }
        }

        isEmptyCompressor.write_to_device();
        inputBlocks.write_to_device();
        inputBlockSizes.write_to_device();
        bitOutBuffers.write_to_device();
        bitOutCnts.write_to_device();
        blocksValuePresent.write_to_device();
        kernel_close->run();
        bitOutBuffers.read_from_device();
        bitOutCnts.read_from_device();

        std::vector<bool> leftBuffer{};

        for (int i = 0; i < blockCompressors.size(); ++i)
        {
            auto &blockCompressor = blockCompressors[i];
            if (!isEmptyCompressor[i])
            {
                writeFileBytes(bitOutBuffers.data() + i * BIT_BLOCK_MAX_SIZE, &(bitOutCnts[i]), outputStream, leftBuffer);
                leftBuffer = getLeftBuffer(bitOutBuffers.data() + i * BIT_BLOCK_MAX_SIZE, &(bitOutCnts[i]));
            }
            blockCompressor.reset();
        }

        // Leftover is written in the next iteration
        writeFileBytes(bitOutBuffers.data(), &(bitOutCnts[0]), outputStream, leftBuffer);
    }
};
#endif