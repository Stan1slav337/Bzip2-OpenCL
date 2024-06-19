/*
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

#ifndef BIT_OUTPUT_STREAM_HPP
#define BIT_OUTPUT_STREAM_HPP

#include <ostream>
#include <bitset>
#include <stdexcept>

class BitOutputStream
{
private:
    static constexpr size_t BLOCK_MAX_BIT_SIZE = 9 * 100000 * 8 * 2;
    std::bitset<BLOCK_MAX_BIT_SIZE> bitBuffer{};
    size_t bitCount = 0UL;

public:
    explicit BitOutputStream() = default;

    std::vector<bool> getLeftBuffer()
    {
        if (bitCount >= 8U)
        {
            throw std::runtime_error("Couldn't write all bytes properly during last step.");
        }

        std::vector<bool> leftBuffer(bitCount);
        for (int i = 0; i < bitCount; ++i)
        {
            leftBuffer[i] = bitBuffer[i];
        }
        bitCount = 0UL;

        return leftBuffer;
    }

    void writeFileBytes(std::ostream &out, const std::vector<bool> &leftBuffer)
    {
        size_t bitsLeft = leftBuffer.size();

        // leftover and current doesn't make a byte
        if (bitsLeft + bitCount < 8UL)
        {
            if (bitCount)
            {
                for (size_t i = bitCount - 1UL; i >= 0UL; --i)
                {
                    bitBuffer[i + bitsLeft] = bitBuffer[i];
                }
            }

            for (size_t i = 0UL; i < bitsLeft; ++i)
            {
                bitBuffer[i] = leftBuffer[i];
            }
            bitCount += bitsLeft;
            return;
        }

        unsigned char byte = 0U;
        for (bool leftBit : leftBuffer)
        {
            byte = (byte << 1) | leftBit;
        }
        size_t offset = 8UL - bitsLeft;
        for (size_t i = 0UL; i < offset; ++i)
        {
            byte = (byte << 1) | bitBuffer[i];
        }
        out.put(byte);

        bitCount -= offset;
        for (size_t i = 0UL; i < bitCount / 8UL; ++i)
        {
            byte = 0U;
            for (int j = 0; j < 8; ++j)
            {
                byte = (byte << 1) | bitBuffer[i * 8UL + j + offset];
            }
            out.put(byte);
        }

        // Shift left-over bits
        for (size_t i = 0UL; i < (bitCount % 8UL); ++i)
        {
            bitBuffer[i] = bitBuffer[i + bitCount - (bitCount % 8) + offset];
        }
        bitCount %= 8UL;
    }

    void writeBoolean(bool value)
    {
        if (bitCount == BLOCK_MAX_BIT_SIZE)
        {
            throw std::runtime_error("Block buffer too low");
        }
        bitBuffer[bitCount++] = value;
    }

    void writeUnary(int value)
    {
        while (value-- > 0)
        {
            writeBoolean(true);
        }
        writeBoolean(false);
    }

    void writeBits(int count, int value)
    {
        for (int bitMask = (1 << (count - 1)); bitMask; bitMask >>= 1)
        {
            writeBoolean(bitMask & value);
        }
    }

    void writeInteger(int value)
    {
        writeBits(16, (value >> 16) & 0xffff);
        writeBits(16, value & 0xffff);
    }

    void padding()
    {
        if (bitCount % 8UL != 0UL)
        {
            writeBits(8 - (bitCount % 8UL), 0);
        }
    }
};
#endif