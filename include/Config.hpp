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

#ifndef CONFIG_HPP
#define CONFIG_HPP

static constexpr int BLOCK_HEADER_MARKER_1 = 0x314159;
static constexpr int BLOCK_HEADER_MARKER_2 = 0x265359;
static constexpr int ALPHABET_SIZE = 256;
static constexpr int BLOCKSIZE_DEFAULT = 10000;
static constexpr int MAX_BLOCK_SIZE = BLOCKSIZE_DEFAULT * 9;
static constexpr int BWT_BUCKET_A_SIZE = 256;
static constexpr int BWT_BUCKET_B_SIZE = 65536;
static constexpr int HUFFMAN_GROUP_RUN_LENGTH = 50;
static constexpr int HUFFMAN_MAXIMUM_ALPHABET_SIZE = 258;
static constexpr int HUFFMAN_HIGH_SYMBOL_COST = 15;
static constexpr int HUFFMAN_ENCODE_MAXIMUM_CODE_LENGTH = 20;
static constexpr int HUFFMAN_DECODE_MAXIMUM_CODE_LENGTH = 23;
static constexpr int HUFFMAN_MINIMUM_TABLES = 2;
static constexpr int HUFFMAN_MAXIMUM_TABLES = 6;
static constexpr int HUFFMAN_MAXIMUM_SELECTORS = (MAX_BLOCK_SIZE / HUFFMAN_GROUP_RUN_LENGTH) + 1;
static constexpr int HUFFMAN_SYMBOL_RUNA = 0;
static constexpr int HUFFMAN_SYMBOL_RUNB = 1;
static constexpr int STREAM_END_MARKER_1 = 0x177245;
static constexpr int STREAM_END_MARKER_2 = 0x385090;
static constexpr int STREAM_START_MARKER_1 = 0x425a;
static constexpr int STREAM_START_MARKER_2 = 0x68;

#endif