/*
 * Copyright (c) 2003-2008 Yuta Mori All Rights Reserved.
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

#include "include/kernel.hpp"

string opencl_c_container()
{
	return R( // ########################## begin of OpenCL C code ####################################################################

			   constant int BLOCK_HEADER_MARKER_1 = 0x314159;
			   constant int BLOCK_HEADER_MARKER_2 = 0x265359;
			   constant int ALPHABET_SIZE = 256;
			   constant int BLOCKSIZE_DEFAULT = 10000;
			   constant int MAX_BLOCK_SIZE = BLOCKSIZE_DEFAULT * 9;
			   constant int HUFFMAN_GROUP_RUN_LENGTH = 50;
			   constant int HUFFMAN_MAXIMUM_ALPHABET_SIZE = 258;
			   constant int HUFFMAN_HIGH_SYMBOL_COST = 15;
			   constant int HUFFMAN_ENCODE_MAXIMUM_CODE_LENGTH = 20;
			   constant int HUFFMAN_DECODE_MAXIMUM_CODE_LENGTH = 23;
			   constant int HUFFMAN_MINIMUM_TABLES = 2;
			   constant int HUFFMAN_MAXIMUM_TABLES = 6;
			   constant int HUFFMAN_MAXIMUM_SELECTORS = (MAX_BLOCK_SIZE / HUFFMAN_GROUP_RUN_LENGTH) + 1;
			   constant int HUFFMAN_SYMBOL_RUNA = 0;
			   constant int HUFFMAN_SYMBOL_RUNB = 1;) +
		   R(/* BWT part */
			 constant int STACK_SIZE = 64;
			 constant int BUCKET_A_SIZE = 256;
			 constant int BUCKET_B_SIZE = 65536;
			 constant int SS_BLOCKSIZE = 1024;
			 constant int INSERTIONSORT_THRESHOLD = 8;
			 constant int log2table[] = {-1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
										 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
										 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
										 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
										 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
										 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
										 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
										 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

			 void swapElements(global int *array1, int index1, global int *array2, int index2) {
				 int temp = array1[index1];
				 array1[index1] = array2[index2];
				 array2[index2] = temp;
			 }

			 int ssCompare(global unsigned char *T, global int *SA, int n, int p1, int p2, int depth) {
				 int U1n, U2n; // pointers within T
				 int U1, U2;

				 for (
					 U1 = depth + SA[p1], U2 = depth + SA[p2], U1n = SA[p1 + 1] + 2, U2n = SA[p2 + 1] + 2;
					 (U1 < U1n) && (U2 < U2n) && (T[U1] == T[U2]);
					 ++U1, ++U2)
					 ;

				 return U1 < U1n ? (U2 < U2n ? (T[U1] & 0xff) - (T[U2] & 0xff) : 1)
								 : (U2 < U2n ? -1 : 0);
			 }

			 int ssCompareLast(global unsigned char *T, global int *SA, int n, int PA, int p1, int p2, int depth, int size) {
				 int U1, U2, U1n, U2n;

				 for (
					 U1 = depth + SA[p1], U2 = depth + SA[p2], U1n = size, U2n = SA[(p2 + 1)] + 2;
					 (U1 < U1n) && (U2 < U2n) && (T[U1] == T[U2]);
					 ++U1, ++U2)
					 ;

				 if (U1 < U1n)
				 {
					 return (U2 < U2n) ? (T[U1] & 0xff) - (T[U2] & 0xff) : 1;
				 }
				 else if (U2 == U2n)
				 {
					 return 1;
				 }

				 for (
					 U1 = U1 % size, U1n = SA[PA] + 2;
					 (U1 < U1n) && (U2 < U2n) && (T[U1] == T[U2]);
					 ++U1, ++U2)
					 ;

				 return U1 < U1n ? (U2 < U2n ? (T[U1] & 0xff) - (T[U2] & 0xff) : 1)
								 : (U2 < U2n ? -1 : 0);
			 }

			 void ssInsertionSort(global unsigned char *T, global int *SA, int n, int PA, int first, int last, int depth) {
				 int i, j; // pointer within SA
				 int t;
				 int r;

				 for (i = last - 2; first <= i; --i)
				 {
					 for (t = SA[i], j = i + 1; 0 < (r = ssCompare(T, SA, n, PA + t, PA + SA[j], depth));)
					 {
						 do
						 {
							 SA[j - 1] = SA[j];
						 } while ((++j < last) && (SA[j] < 0));
						 if (last <= j)
						 {
							 break;
						 }
					 }
					 if (r == 0)
					 {
						 SA[j] = ~SA[j];
					 }
					 SA[j - 1] = t;
				 }
			 }

			 void ssFixdown(global unsigned char *T, global int *SA, int n, int Td, int PA, int sa, int i, int size) {
				 int j, k;
				 int v;
				 int c, d, e;

				 for (v = SA[sa + i], c = (T[Td + SA[PA + v]]) & 0xff; (j = 2 * i + 1) < size; SA[sa + i] = SA[sa + k], i = k)
				 {
					 d = T[Td + SA[PA + SA[sa + (k = j++)]]] & 0xff;
					 if (d < (e = T[Td + SA[PA + SA[sa + j]]] & 0xff))
					 {
						 k = j;
						 d = e;
					 }
					 if (d <= c)
						 break;
				 }
				 SA[sa + i] = v;
			 }

			 void ssHeapSort(global unsigned char *T, global int *SA, int n, int Td, int PA, int sa, int size) {
				 int i, m;
				 int t;

				 m = size;
				 if ((size % 2) == 0)
				 {
					 m--;
					 if ((T[Td + SA[PA + SA[sa + (m / 2)]]] & 0xff) < (T[Td + SA[PA + SA[sa + m]]] & 0xff))
					 {
						 swapElements(SA, sa + m, SA, sa + (m / 2));
					 }
				 }

				 for (i = m / 2 - 1; 0 <= i; --i)
				 {
					 ssFixdown(T, SA, n, Td, PA, sa, i, m);
				 }

				 if ((size % 2) == 0)
				 {
					 swapElements(SA, sa, SA, sa + m);
					 ssFixdown(T, SA, n, Td, PA, sa, 0, m);
				 }

				 for (i = m - 1; 0 < i; --i)
				 {
					 t = SA[sa];
					 SA[sa] = SA[sa + i];
					 ssFixdown(T, SA, n, Td, PA, sa, 0, i);
					 SA[sa + i] = t;
				 }
			 }

			 int ssMedian3(global unsigned char *T, global int *SA, int n, int Td, int PA, int v1, int v2, int v3) {
				 int T_v1 = T[Td + SA[PA + SA[v1]]] & 0xff;
				 int T_v2 = T[Td + SA[PA + SA[v2]]] & 0xff;
				 int T_v3 = T[Td + SA[PA + SA[v3]]] & 0xff;

				 if (T_v1 > T_v2)
				 {
					 int temp = v1;
					 v1 = v2;
					 v2 = temp;
					 int T_vtemp = T_v1;
					 T_v1 = T_v2;
					 T_v2 = T_vtemp;
				 }
				 if (T_v2 > T_v3)
				 {
					 if (T_v1 > T_v3)
					 {
						 return v1;
					 }
					 return v3;
				 }
				 return v2;
			 }

			 int ssMedian5(global unsigned char *T, global int *SA, int n, int Td, int PA, int v1, int v2, int v3, int v4, int v5) {
				 int T_v1 = T[Td + SA[PA + SA[v1]]] & 0xff;
				 int T_v2 = T[Td + SA[PA + SA[v2]]] & 0xff;
				 int T_v3 = T[Td + SA[PA + SA[v3]]] & 0xff;
				 int T_v4 = T[Td + SA[PA + SA[v4]]] & 0xff;
				 int T_v5 = T[Td + SA[PA + SA[v5]]] & 0xff;
				 int temp;
				 int T_vtemp;

				 if (T_v2 > T_v3)
				 {
					 temp = v2;
					 v2 = v3;
					 v3 = temp;
					 T_vtemp = T_v2;
					 T_v2 = T_v3;
					 T_v3 = T_vtemp;
				 }
				 if (T_v4 > T_v5)
				 {
					 temp = v4;
					 v4 = v5;
					 v5 = temp;
					 T_vtemp = T_v4;
					 T_v4 = T_v5;
					 T_v5 = T_vtemp;
				 }
				 if (T_v2 > T_v4)
				 {
					 temp = v2;
					 v2 = v4;
					 v4 = temp;
					 T_vtemp = T_v2;
					 T_v2 = T_v4;
					 T_v4 = T_vtemp;
					 temp = v3;
					 v3 = v5;
					 v5 = temp;
					 T_vtemp = T_v3;
					 T_v3 = T_v5;
					 T_v5 = T_vtemp;
				 }
				 if (T_v1 > T_v3)
				 {
					 temp = v1;
					 v1 = v3;
					 v3 = temp;
					 T_vtemp = T_v1;
					 T_v1 = T_v3;
					 T_v3 = T_vtemp;
				 }
				 if (T_v1 > T_v4)
				 {
					 temp = v1;
					 v1 = v4;
					 v4 = temp;
					 T_vtemp = T_v1;
					 T_v1 = T_v4;
					 T_v4 = T_vtemp;
					 temp = v3;
					 v3 = v5;
					 v5 = temp;
					 T_vtemp = T_v3;
					 T_v3 = T_v5;
					 T_v5 = T_vtemp;
				 }
				 if (T_v3 > T_v4)
				 {
					 return v4;
				 }
				 return v3;
			 }

			 int ssPivot(global unsigned char *T, global int *SA, int n, int Td, int PA, int first, int last) {
				 int middle;
				 int t;

				 t = last - first;
				 middle = first + t / 2;

				 if (t <= 512)
				 {
					 if (t <= 32)
					 {
						 return ssMedian3(T, SA, n, Td, PA, first, middle, last - 1);
					 }
					 t >>= 2;
					 return ssMedian5(T, SA, n, Td, PA, first, first + t, middle, last - 1 - t, last - 1);
				 }
				 t >>= 3;
				 return ssMedian3(T, SA, n,
								  Td, PA,
								  ssMedian3(T, SA, n, Td, PA, first, first + t, first + (t << 1)),
								  ssMedian3(T, SA, n, Td, PA, middle - t, middle, middle + t),
								  ssMedian3(T, SA, n, Td, PA, last - 1 - (t << 1), last - 1 - t, last - 1));
			 }

			 int ssLog(int n) {
				 return ((n & 0xff00) != 0) ? 8 + log2table[(n >> 8) & 0xff]
											: log2table[n & 0xff];
			 }

			 int ssSubstringPartition(global unsigned char *T, global int *SA, int n, int PA, int first, int last, int depth) {
				 int a, b;
				 int t;

				 for (a = first - 1, b = last;;)
				 {
					 for (; (++a < b) && ((SA[PA + SA[a]] + depth) >= (SA[PA + SA[a] + 1] + 1));)
					 {
						 SA[a] = ~SA[a];
					 }
					 for (; (a < --b) && ((SA[PA + SA[b]] + depth) < (SA[PA + SA[b] + 1] + 1));)
						 ;
					 if (b <= a)
					 {
						 break;
					 }
					 t = ~SA[b];
					 SA[b] = SA[a];
					 SA[a] = t;
				 }
				 if (first < a)
				 {
					 SA[first] = ~SA[first];
				 }

				 return a;
			 }

			 struct StackEntry {
				 int a;
				 int b;
				 int c;
				 int d;
			 };

			 struct StackEntry createStackEntry(int a, int b, int c, int d) {
				 struct StackEntry entry = {a, b, c, d};
				 return entry;
			 }

			 void ssMultiKeyIntroSort(global unsigned char *T, global int *SA, int n, int PA, int first, int last, int depth) {
				 struct StackEntry stack[STACK_SIZE] = {{0, 0, 0, 0}};

				 int Td = 0;
				 int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
				 int s = 0, t = 0;
				 int ssize;
				 int limit;
				 int v = 0, x = 0;

				 for (ssize = 0, limit = ssLog(last - first);;)
				 {
					 if ((last - first) <= INSERTIONSORT_THRESHOLD)
					 {
						 if (1 < (last - first))
						 {
							 ssInsertionSort(T, SA, n, PA, first, last, depth);
						 }
						 if (ssize == 0)
							 return;
						 struct StackEntry entry = stack[--ssize];
						 first = entry.a;
						 last = entry.b;
						 depth = entry.c;
						 limit = entry.d;
						 continue;
					 }

					 Td = depth;
					 if (limit-- == 0)
					 {
						 ssHeapSort(T, SA, n, Td, PA, first, last - first);
					 }
					 if (limit < 0)
					 {
						 for (a = first + 1, v = T[Td + SA[PA + SA[first]]] & 0xff; a < last; ++a)
						 {
							 if ((x = (T[Td + SA[PA + SA[a]]] & 0xff)) != v)
							 {
								 if (1 < (a - first))
								 {
									 break;
								 }
								 v = x;
								 first = a;
							 }
						 }
						 if ((T[Td + SA[PA + SA[first]] - 1] & 0xff) < v)
						 {
							 first = ssSubstringPartition(T, SA, n, PA, first, a, depth);
						 }
						 if ((a - first) <= (last - a))
						 {
							 if (1 < (a - first))
							 {
								 stack[ssize++] = createStackEntry(a, last, depth, -1);
								 last = a;
								 depth += 1;
								 limit = ssLog(a - first);
							 }
							 else
							 {
								 first = a;
								 limit = -1;
							 }
						 }
						 else
						 {
							 if (1 < (last - a))
							 {
								 stack[ssize++] = createStackEntry(first, a, depth + 1, ssLog(a - first));
								 first = a;
								 limit = -1;
							 }
							 else
							 {
								 last = a;
								 depth += 1;
								 limit = ssLog(a - first);
							 }
						 }
						 continue;
					 }

					 a = ssPivot(T, SA, n, Td, PA, first, last);
					 v = T[Td + SA[PA + SA[a]]] & 0xff;
					 swapElements(SA, first, SA, a);

					 for (b = first; (++b < last) && ((x = (T[Td + SA[PA + SA[b]]] & 0xff)) == v);)
						 ;
					 if (((a = b) < last) && (x < v))
					 {
						 for (; (++b < last) && ((x = (T[Td + SA[PA + SA[b]]] & 0xff)) <= v);)
						 {
							 if (x == v)
							 {
								 swapElements(SA, b, SA, a);
								 ++a;
							 }
						 }
					 }
					 for (c = last; (b < --c) && ((x = (T[Td + SA[PA + SA[c]]] & 0xff)) == v);)
						 ;
					 if ((b < (d = c)) && (x > v))
					 {
						 for (; (b < --c) && ((x = (T[Td + SA[PA + SA[c]]] & 0xff)) >= v);)
						 {
							 if (x == v)
							 {
								 swapElements(SA, c, SA, d);
								 --d;
							 }
						 }
					 }
					 for (; b < c;)
					 {
						 swapElements(SA, b, SA, c);
						 for (; (++b < c) && ((x = (T[Td + SA[PA + SA[b]]] & 0xff)) <= v);)
						 {
							 if (x == v)
							 {
								 swapElements(SA, b, SA, a);
								 ++a;
							 }
						 }
						 for (; (b < --c) && ((x = (T[Td + SA[PA + SA[c]]] & 0xff)) >= v);)
						 {
							 if (x == v)
							 {
								 swapElements(SA, c, SA, d);
								 --d;
							 }
						 }
					 }

					 if (a <= d)
					 {
						 c = b - 1;

						 if ((s = a - first) > (t = b - a))
						 {
							 s = t;
						 }
						 for (e = first, f = b - s; 0 < s; --s, ++e, ++f)
						 {
							 swapElements(SA, e, SA, f);
						 }
						 if ((s = d - c) > (t = last - d - 1))
						 {
							 s = t;
						 }
						 for (e = b, f = last - s; 0 < s; --s, ++e, ++f)
						 {
							 swapElements(SA, e, SA, f);
						 }

						 a = first + (b - a);
						 c = last - (d - c);
						 b = (v <= (T[Td + SA[PA + SA[a]] - 1] & 0xff)) ? a : ssSubstringPartition(T, SA, n, PA, a, c, depth);

						 if ((a - first) <= (last - c))
						 {
							 if ((last - c) <= (c - b))
							 {
								 stack[ssize++] = createStackEntry(b, c, depth + 1, ssLog(c - b));
								 stack[ssize++] = createStackEntry(c, last, depth, limit);
								 last = a;
							 }
							 else if ((a - first) <= (c - b))
							 {
								 stack[ssize++] = createStackEntry(c, last, depth, limit);
								 stack[ssize++] = createStackEntry(b, c, depth + 1, ssLog(c - b));
								 last = a;
							 }
							 else
							 {
								 stack[ssize++] = createStackEntry(c, last, depth, limit);
								 stack[ssize++] = createStackEntry(first, a, depth, limit);
								 first = b;
								 last = c;
								 depth += 1;
								 limit = ssLog(c - b);
							 }
						 }
						 else
						 {
							 if ((a - first) <= (c - b))
							 {
								 stack[ssize++] = createStackEntry(b, c, depth + 1, ssLog(c - b));
								 stack[ssize++] = createStackEntry(first, a, depth, limit);
								 first = c;
							 }
							 else if ((last - c) <= (c - b))
							 {
								 stack[ssize++] = createStackEntry(first, a, depth, limit);
								 stack[ssize++] = createStackEntry(b, c, depth + 1, ssLog(c - b));
								 first = c;
							 }
							 else
							 {
								 stack[ssize++] = createStackEntry(first, a, depth, limit);
								 stack[ssize++] = createStackEntry(c, last, depth, limit);
								 first = b;
								 last = c;
								 depth += 1;
								 limit = ssLog(c - b);
							 }
						 }
					 }
					 else
					 {
						 limit += 1;
						 if ((T[Td + SA[PA + SA[first]] - 1] & 0xff) < v)
						 {
							 first = ssSubstringPartition(T, SA, n, PA, first, last, depth);
							 limit = ssLog(last - first);
						 }
						 depth += 1;
					 }
				 }
			 }

			 void ssBlockSwap(global unsigned char *T, global int *SA, int n, global int *array1, int first1, global int *array2, int first2, int size) {
				 int a, b;
				 int i;
				 for (i = size, a = first1, b = first2; 0 < i; --i, ++a, ++b)
				 {
					 swapElements(array1, a, array2, b);
				 }
			 }

			 void ssMergeForward(global unsigned char *T, global int *SA, int n, int PA, global int *buf, int bufoffset, int first, int middle, int last, int depth) {
				 int bufend;
				 int i, j, k;
				 int t;
				 int r;

				 bufend = bufoffset + (middle - first) - 1;
				 ssBlockSwap(T, SA, n, buf, bufoffset, SA, first, middle - first);

				 for (t = SA[first], i = first, j = bufoffset, k = middle;;)
				 {
					 r = ssCompare(T, SA, n, PA + buf[j], PA + SA[k], depth);
					 if (r < 0)
					 {
						 do
						 {
							 SA[i++] = buf[j];
							 if (bufend <= j)
							 {
								 buf[j] = t;
								 return;
							 }
							 buf[j++] = SA[i];
						 } while (buf[j] < 0);
					 }
					 else if (r > 0)
					 {
						 do
						 {
							 SA[i++] = SA[k];
							 SA[k++] = SA[i];
							 if (last <= k)
							 {
								 while (j < bufend)
								 {
									 SA[i++] = buf[j];
									 buf[j++] = SA[i];
								 }
								 SA[i] = buf[j];
								 buf[j] = t;
								 return;
							 }
						 } while (SA[k] < 0);
					 }
					 else
					 {
						 SA[k] = ~SA[k];
						 do
						 {
							 SA[i++] = buf[j];
							 if (bufend <= j)
							 {
								 buf[j] = t;
								 return;
							 }
							 buf[j++] = SA[i];
						 } while (buf[j] < 0);

						 do
						 {
							 SA[i++] = SA[k];
							 SA[k++] = SA[i];
							 if (last <= k)
							 {
								 while (j < bufend)
								 {
									 SA[i++] = buf[j];
									 buf[j++] = SA[i];
								 }
								 SA[i] = buf[j];
								 buf[j] = t;
								 return;
							 }
						 } while (SA[k] < 0);
					 }
				 }
			 }) +
		   R(
			   void ssMergeBackward(global unsigned char *T, global int *SA, int n, int PA, global int *buf, int bufoffset, int first, int middle, int last, int depth) {
				   int p1, p2;
				   int bufend;
				   int i, j, k;
				   int t;
				   int r;
				   int x;

				   bufend = bufoffset + (last - middle);
				   ssBlockSwap(T, SA, n, buf, bufoffset, SA, middle, last - middle);

				   x = 0;
				   if (buf[bufend - 1] < 0)
				   {
					   x |= 1;
					   p1 = PA + ~buf[bufend - 1];
				   }
				   else
				   {
					   p1 = PA + buf[bufend - 1];
				   }
				   if (SA[middle - 1] < 0)
				   {
					   x |= 2;
					   p2 = PA + ~SA[middle - 1];
				   }
				   else
				   {
					   p2 = PA + SA[middle - 1];
				   }
				   for (t = SA[last - 1], i = last - 1, j = bufend - 1, k = middle - 1;;)
				   {

					   r = ssCompare(T, SA, n, p1, p2, depth);
					   if (r > 0)
					   {
						   if ((x & 1) != 0)
						   {
							   do
							   {
								   SA[i--] = buf[j];
								   buf[j--] = SA[i];
							   } while (buf[j] < 0);
							   x ^= 1;
						   }
						   SA[i--] = buf[j];
						   if (j <= bufoffset)
						   {
							   buf[j] = t;
							   return;
						   }
						   buf[j--] = SA[i];

						   if (buf[j] < 0)
						   {
							   x |= 1;
							   p1 = PA + ~buf[j];
						   }
						   else
						   {
							   p1 = PA + buf[j];
						   }
					   }
					   else if (r < 0)
					   {
						   if ((x & 2) != 0)
						   {
							   do
							   {
								   SA[i--] = SA[k];
								   SA[k--] = SA[i];
							   } while (SA[k] < 0);
							   x ^= 2;
						   }
						   SA[i--] = SA[k];
						   SA[k--] = SA[i];
						   if (k < first)
						   {
							   while (bufoffset < j)
							   {
								   SA[i--] = buf[j];
								   buf[j--] = SA[i];
							   }
							   SA[i] = buf[j];
							   buf[j] = t;
							   return;
						   }

						   if (SA[k] < 0)
						   {
							   x |= 2;
							   p2 = PA + ~SA[k];
						   }
						   else
						   {
							   p2 = PA + SA[k];
						   }
					   }
					   else
					   {
						   if ((x & 1) != 0)
						   {
							   do
							   {
								   SA[i--] = buf[j];
								   buf[j--] = SA[i];
							   } while (buf[j] < 0);
							   x ^= 1;
						   }
						   SA[i--] = ~buf[j];
						   if (j <= bufoffset)
						   {
							   buf[j] = t;
							   return;
						   }
						   buf[j--] = SA[i];

						   if ((x & 2) != 0)
						   {
							   do
							   {
								   SA[i--] = SA[k];
								   SA[k--] = SA[i];
							   } while (SA[k] < 0);
							   x ^= 2;
						   }
						   SA[i--] = SA[k];
						   SA[k--] = SA[i];
						   if (k < first)
						   {
							   while (bufoffset < j)
							   {
								   SA[i--] = buf[j];
								   buf[j--] = SA[i];
							   }
							   SA[i] = buf[j];
							   buf[j] = t;
							   return;
						   }

						   if (buf[j] < 0)
						   {
							   x |= 1;
							   p1 = PA + ~buf[j];
						   }
						   else
						   {
							   p1 = PA + buf[j];
						   }
						   if (SA[k] < 0)
						   {
							   x |= 2;
							   p2 = PA + ~SA[k];
						   }
						   else
						   {
							   p2 = PA + SA[k];
						   }
					   }
				   }
			   }

			   int getIDX(int a) {
				   return (0 <= a) ? a : ~a;
			   }

			   void ssMergeCheckEqual(global unsigned char *T, global int *SA, int n, int PA, int depth, int a) {
				   if (
					   (0 <= SA[a]) && (ssCompare(T, SA, n, PA + getIDX(SA[a - 1]), PA + SA[a], depth) == 0))
				   {
					   SA[a] = ~SA[a];
				   }
			   }

			   void ssMerge(global unsigned char *T, global int *SA, int n, int PA, int first, int middle, int last, global int *buf, int bufoffset, int bufsize, int depth) {
				   struct StackEntry stack[STACK_SIZE] = {{0, 0, 0, 0}};
				   int i, j;
				   int m, len, halfVAR;
				   int ssize;
				   int check, next;

				   for (check = 0, ssize = 0;;)
				   {
					   if ((last - middle) <= bufsize)
					   {
						   if ((first < middle) && (middle < last))
						   {
							   ssMergeBackward(T, SA, n, PA, buf, bufoffset, first, middle, last, depth);
						   }

						   if ((check & 1) != 0)
						   {
							   ssMergeCheckEqual(T, SA, n, PA, depth, first);
						   }
						   if ((check & 2) != 0)
						   {
							   ssMergeCheckEqual(T, SA, n, PA, depth, last);
						   }
						   if (ssize == 0)
							   return;
						   struct StackEntry entry = stack[--ssize];
						   first = entry.a;
						   middle = entry.b;
						   last = entry.c;
						   check = entry.d;
						   continue;
					   }

					   if ((middle - first) <= bufsize)
					   {
						   if (first < middle)
						   {
							   ssMergeForward(T, SA, n, PA, buf, bufoffset, first, middle, last, depth);
						   }
						   if ((check & 1) != 0)
						   {
							   ssMergeCheckEqual(T, SA, n, PA, depth, first);
						   }
						   if ((check & 2) != 0)
						   {
							   ssMergeCheckEqual(T, SA, n, PA, depth, last);
						   }
						   if (ssize == 0)
							   return;
						   struct StackEntry entry = stack[--ssize];
						   first = entry.a;
						   middle = entry.b;
						   last = entry.c;
						   check = entry.d;
						   continue;
					   }

					   for (
						   m = 0, len = middle - first < last - middle ? middle - first : last - middle, halfVAR = len >> 1;
						   0 < len;
						   len = halfVAR, halfVAR >>= 1)
					   {
						   if (ssCompare(T, SA, n, PA + getIDX(SA[middle + m + halfVAR]),
										 PA + getIDX(SA[middle - m - halfVAR - 1]), depth) < 0)
						   {
							   m += halfVAR + 1;
							   halfVAR -= (len & 1) ^ 1;
						   }
					   }

					   if (0 < m)
					   {
						   ssBlockSwap(T, SA, n, SA, middle - m, SA, middle, m);
						   i = j = middle;
						   next = 0;
						   if ((middle + m) < last)
						   {
							   if (SA[middle + m] < 0)
							   {
								   for (; SA[i - 1] < 0; --i)
									   ;
								   SA[middle + m] = ~SA[middle + m];
							   }
							   for (j = middle; SA[j] < 0; ++j)
								   ;
							   next = 1;
						   }
						   if ((i - first) <= (last - j))
						   {
							   stack[ssize++] = createStackEntry(j, middle + m, last, (check & 2) | (next & 1));
							   middle -= m;
							   last = i;
							   check = (check & 1);
						   }
						   else
						   {
							   if ((i == middle) && (middle == j))
							   {
								   next <<= 1;
							   }
							   stack[ssize++] = createStackEntry(first, middle - m, i, (check & 1) | (next & 2));
							   first = j;
							   middle += m;
							   check = (check & 2) | (next & 1);
						   }
					   }
					   else
					   {
						   if ((check & 1) != 0)
						   {
							   ssMergeCheckEqual(T, SA, n, PA, depth, first);
						   }
						   ssMergeCheckEqual(T, SA, n, PA, depth, middle);
						   if ((check & 2) != 0)
						   {
							   ssMergeCheckEqual(T, SA, n, PA, depth, last);
						   }
						   if (ssize == 0)
							   return;
						   struct StackEntry entry = stack[--ssize];
						   first = entry.a;
						   middle = entry.b;
						   last = entry.c;
						   check = entry.d;
					   }
				   }
			   }

			   void subStringSort(global unsigned char *T, global int *SA, int n, int PA, int first, int last, global int *buf, int bufoffset, int bufsize, int depth, bool lastsuffix, int size) {
				   int a, b;
				   int curbufoffset;
				   int i, j, k;
				   int curbufsize;

				   if (lastsuffix)
				   {
					   ++first;
				   }
				   for (a = first, i = 0; (a + SS_BLOCKSIZE) < last; a += SS_BLOCKSIZE, ++i)
				   {
					   ssMultiKeyIntroSort(T, SA, n, PA, a, a + SS_BLOCKSIZE, depth);
					   global int *curbuf = SA;
					   curbufoffset = a + SS_BLOCKSIZE;
					   curbufsize = last - (a + SS_BLOCKSIZE);
					   if (curbufsize <= bufsize)
					   {
						   curbufsize = bufsize;
						   curbuf = buf;
						   curbufoffset = bufoffset;
					   }
					   for (b = a, k = SS_BLOCKSIZE, j = i; (j & 1) != 0; b -= k, k <<= 1, j >>= 1)
					   {
						   ssMerge(T, SA, n, PA, b - k, b, b + k, curbuf, curbufoffset, curbufsize, depth);
					   }
				   }

				   ssMultiKeyIntroSort(T, SA, n, PA, a, last, depth);

				   for (k = SS_BLOCKSIZE; i != 0; k <<= 1, i >>= 1)
				   {
					   if ((i & 1) != 0)
					   {
						   ssMerge(T, SA, n, PA, a - k, a, last, buf, bufoffset, bufsize, depth);
						   a -= k;
					   }
				   }

				   if (lastsuffix)
				   {
					   int r;
					   for (
						   a = first, i = SA[first - 1], r = 1;
						   (a < last) && ((SA[a] < 0) || (0 < (r = ssCompareLast(T, SA, n, PA, PA + i, PA + SA[a], depth, size))));
						   ++a)
					   {
						   SA[a - 1] = SA[a];
					   }
					   if (r == 0)
					   {
						   SA[a] = ~SA[a];
					   }
					   SA[a - 1] = i;
				   }
			   }

			   int trGetC(global unsigned char *T, global int *SA, int n, int ISA, int ISAd, int ISAn, int p) {
				   return (((ISAd + p) < ISAn) ? SA[ISAd + p] : SA[ISA + ((ISAd - ISA + p) % (ISAn - ISA))]);
			   }

			   void trFixdown(global unsigned char *T, global int *SA, int n, int ISA, int ISAd, int ISAn, int sa, int i, int size) {
				   int j, k;
				   int v;
				   int c, d, e;

				   for (v = SA[sa + i], c = trGetC(T, SA, n, ISA, ISAd, ISAn, v); (j = 2 * i + 1) < size; SA[sa + i] = SA[sa + k], i = k)
				   {
					   k = j++;
					   d = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[sa + k]);
					   if (d < (e = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[sa + j])))
					   {
						   k = j;
						   d = e;
					   }
					   if (d <= c)
					   {
						   break;
					   }
				   }
				   SA[sa + i] = v;
			   }

			   void trHeapSort(global unsigned char *T, global int *SA, int n, int ISA, int ISAd, int ISAn, int sa, int size) {
				   int i, m;
				   int t;

				   m = size;
				   if ((size % 2) == 0)
				   {
					   m--;
					   if (trGetC(T, SA, n, ISA, ISAd, ISAn, SA[sa + (m / 2)]) < trGetC(T, SA, n, ISA, ISAd, ISAn, SA[sa + m]))
					   {
						   swapElements(SA, sa + m, SA, sa + (m / 2));
					   }
				   }

				   for (i = m / 2 - 1; 0 <= i; --i)
				   {
					   trFixdown(T, SA, n, ISA, ISAd, ISAn, sa, i, m);
				   }

				   if ((size % 2) == 0)
				   {
					   swapElements(SA, sa + 0, SA, sa + m);
					   trFixdown(T, SA, n, ISA, ISAd, ISAn, sa, 0, m);
				   }

				   for (i = m - 1; 0 < i; --i)
				   {
					   t = SA[sa + 0];
					   SA[sa + 0] = SA[sa + i];
					   trFixdown(T, SA, n, ISA, ISAd, ISAn, sa, 0, i);
					   SA[sa + i] = t;
				   }
			   }

			   void trInsertionSort(global unsigned char *T, global int *SA, int n, int ISA, int ISAd, int ISAn, int first, int last) {
				   int a, b;
				   int t, r;

				   for (a = first + 1; a < last; ++a)
				   {
					   for (t = SA[a], b = a - 1; 0 > (r = trGetC(T, SA, n, ISA, ISAd, ISAn, t) - trGetC(T, SA, n, ISA, ISAd, ISAn, SA[b]));)
					   {
						   do
						   {
							   SA[b + 1] = SA[b];
						   } while ((first <= --b) && (SA[b] < 0));
						   if (b < first)
						   {
							   break;
						   }
					   }
					   if (r == 0)
					   {
						   SA[b] = ~SA[b];
					   }
					   SA[b + 1] = t;
				   }
			   }

			   int trLog(int n) {
				   return ((n & 0xffff0000) != 0) ? (((n & 0xff000000) != 0) ? 24 + log2table[(n >> 24) & 0xff] : 16 + log2table[(n >> 16) & 0xff])
												  : (((n & 0x0000ff00) != 0) ? 8 + log2table[(n >> 8) & 0xff] : 0 + log2table[(n >> 0) & 0xff]);
			   }) +
		   R(
			   int trMedian3(global unsigned char *T, global int *SA, int n, int ISA, int ISAd, int ISAn, int v1, int v2, int v3) {
				   int SA_v1 = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[v1]);
				   int SA_v2 = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[v2]);
				   int SA_v3 = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[v3]);

				   if (SA_v1 > SA_v2)
				   {
					   int temp = v1;
					   v1 = v2;
					   v2 = temp;
					   int SA_vtemp = SA_v1;
					   SA_v1 = SA_v2;
					   SA_v2 = SA_vtemp;
				   }
				   if (SA_v2 > SA_v3)
				   {
					   if (SA_v1 > SA_v3)
					   {
						   return v1;
					   }
					   return v3;
				   }

				   return v2;
			   }

			   int trMedian5(global unsigned char *T, global int *SA, int n, int ISA, int ISAd, int ISAn, int v1, int v2, int v3, int v4, int v5) {
				   int SA_v1 = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[v1]);
				   int SA_v2 = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[v2]);
				   int SA_v3 = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[v3]);
				   int SA_v4 = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[v4]);
				   int SA_v5 = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[v5]);
				   int temp;
				   int SA_vtemp;

				   if (SA_v2 > SA_v3)
				   {
					   temp = v2;
					   v2 = v3;
					   v3 = temp;
					   SA_vtemp = SA_v2;
					   SA_v2 = SA_v3;
					   SA_v3 = SA_vtemp;
				   }
				   if (SA_v4 > SA_v5)
				   {
					   temp = v4;
					   v4 = v5;
					   v5 = temp;
					   SA_vtemp = SA_v4;
					   SA_v4 = SA_v5;
					   SA_v5 = SA_vtemp;
				   }
				   if (SA_v2 > SA_v4)
				   {
					   temp = v2;
					   v2 = v4;
					   v4 = temp;
					   SA_vtemp = SA_v2;
					   SA_v2 = SA_v4;
					   SA_v4 = SA_vtemp;
					   temp = v3;
					   v3 = v5;
					   v5 = temp;
					   SA_vtemp = SA_v3;
					   SA_v3 = SA_v5;
					   SA_v5 = SA_vtemp;
				   }
				   if (SA_v1 > SA_v3)
				   {
					   temp = v1;
					   v1 = v3;
					   v3 = temp;
					   SA_vtemp = SA_v1;
					   SA_v1 = SA_v3;
					   SA_v3 = SA_vtemp;
				   }
				   if (SA_v1 > SA_v4)
				   {
					   temp = v1;
					   v1 = v4;
					   v4 = temp;
					   SA_vtemp = SA_v1;
					   SA_v1 = SA_v4;
					   SA_v4 = SA_vtemp;
					   temp = v3;
					   v3 = v5;
					   v5 = temp;
					   SA_vtemp = SA_v3;
					   SA_v3 = SA_v5;
					   SA_v5 = SA_vtemp;
				   }
				   if (SA_v3 > SA_v4)
				   {
					   return v4;
				   }
				   return v3;
			   }

			   int trPivot(global unsigned char *T, global int *SA, int n, int ISA, int ISAd, int ISAn, int first, int last) {
				   int middle;
				   int t;

				   t = last - first;
				   middle = first + t / 2;

				   if (t <= 512)
				   {
					   if (t <= 32)
					   {
						   return trMedian3(T, SA, n, ISA, ISAd, ISAn, first, middle, last - 1);
					   }
					   t >>= 2;
					   return trMedian5(T, SA, n,
										ISA, ISAd, ISAn,
										first, first + t,
										middle,
										last - 1 - t, last - 1);
				   }
				   t >>= 3;
				   return trMedian3(T, SA, n,
									ISA, ISAd, ISAn,
									trMedian3(T, SA, n, ISA, ISAd, ISAn, first, first + t, first + (t << 1)),
									trMedian3(T, SA, n, ISA, ISAd, ISAn, middle - t, middle, middle + t),
									trMedian3(T, SA, n, ISA, ISAd, ISAn, last - 1 - (t << 1), last - 1 - t, last - 1));
			   }

			   void lsUpdateGroup(global unsigned char *T, global int *SA, int n, int ISA, int first, int last) {
				   int a, b;
				   int t;

				   for (a = first; a < last; ++a)
				   {
					   if (0 <= SA[a])
					   {
						   b = a;
						   do
						   {
							   SA[ISA + SA[a]] = a;
						   } while ((++a < last) && (0 <= SA[a]));
						   SA[b] = b - a;
						   if (last <= a)
						   {
							   break;
						   }
					   }
					   b = a;
					   do
					   {
						   SA[a] = ~SA[a];
					   } while (SA[++a] < 0);
					   t = a;
					   do
					   {
						   SA[ISA + SA[b]] = t;
					   } while (++b <= a);
				   }
			   }

			   void lsIntroSort(global unsigned char *T, global int *SA, int n, int ISA, int ISAd, int ISAn, int first, int last) {
				   struct StackEntry stack[STACK_SIZE] = {{0, 0, 0, 0}};
				   int a, b, c, d, e, f;
				   int s, t;
				   int limit;
				   int v, x = 0;
				   int ssize;

				   for (ssize = 0, limit = trLog(last - first);;)
				   {

					   if ((last - first) <= INSERTIONSORT_THRESHOLD)
					   {
						   if (1 < (last - first))
						   {
							   trInsertionSort(T, SA, n, ISA, ISAd, ISAn, first, last);
							   lsUpdateGroup(T, SA, n, ISA, first, last);
						   }
						   else if ((last - first) == 1)
						   {
							   SA[first] = -1;
						   }
						   if (ssize == 0)
							   return;
						   struct StackEntry entry = stack[--ssize];
						   first = entry.a;
						   last = entry.b;
						   limit = entry.c;
						   continue;
					   }

					   if (limit-- == 0)
					   {
						   trHeapSort(T, SA, n, ISA, ISAd, ISAn, first, last - first);
						   for (a = last - 1; first < a; a = b)
						   {
							   for (
								   x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[a]), b = a - 1;
								   (first <= b) && (trGetC(T, SA, n, ISA, ISAd, ISAn, SA[b]) == x);
								   --b)
							   {
								   SA[b] = ~SA[b];
							   }
						   }
						   lsUpdateGroup(T, SA, n, ISA, first, last);
						   if (ssize == 0)
							   return;
						   struct StackEntry entry = stack[--ssize];
						   first = entry.a;
						   last = entry.b;
						   limit = entry.c;
						   continue;
					   }

					   a = trPivot(T, SA, n, ISA, ISAd, ISAn, first, last);
					   swapElements(SA, first, SA, a);
					   v = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[first]);

					   for (b = first; (++b < last) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[b])) == v);)
						   ;
					   if (((a = b) < last) && (x < v))
					   {
						   for (; (++b < last) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[b])) <= v);)
						   {
							   if (x == v)
							   {
								   swapElements(SA, b, SA, a);
								   ++a;
							   }
						   }
					   }
					   for (c = last; (b < --c) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[c])) == v);)
						   ;
					   if ((b < (d = c)) && (x > v))
					   {
						   for (; (b < --c) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[c])) >= v);)
						   {
							   if (x == v)
							   {
								   swapElements(SA, c, SA, d);
								   --d;
							   }
						   }
					   }
					   for (; b < c;)
					   {
						   swapElements(SA, b, SA, c);
						   for (; (++b < c) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[b])) <= v);)
						   {
							   if (x == v)
							   {
								   swapElements(SA, b, SA, a);
								   ++a;
							   }
						   }
						   for (; (b < --c) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[c])) >= v);)
						   {
							   if (x == v)
							   {
								   swapElements(SA, c, SA, d);
								   --d;
							   }
						   }
					   }

					   if (a <= d)
					   {
						   c = b - 1;

						   if ((s = a - first) > (t = b - a))
						   {
							   s = t;
						   }
						   for (e = first, f = b - s; 0 < s; --s, ++e, ++f)
						   {
							   swapElements(SA, e, SA, f);
						   }
						   if ((s = d - c) > (t = last - d - 1))
						   {
							   s = t;
						   }
						   for (e = b, f = last - s; 0 < s; --s, ++e, ++f)
						   {
							   swapElements(SA, e, SA, f);
						   }

						   a = first + (b - a);
						   b = last - (d - c);

						   for (c = first, v = a - 1; c < a; ++c)
						   {
							   SA[ISA + SA[c]] = v;
						   }
						   if (b < last)
						   {
							   for (c = a, v = b - 1; c < b; ++c)
							   {
								   SA[ISA + SA[c]] = v;
							   }
						   }
						   if ((b - a) == 1)
						   {
							   SA[a] = -1;
						   }

						   if ((a - first) <= (last - b))
						   {
							   if (first < a)
							   {
								   stack[ssize++] = createStackEntry(b, last, limit, 0);
								   last = a;
							   }
							   else
							   {
								   first = b;
							   }
						   }
						   else
						   {
							   if (b < last)
							   {
								   stack[ssize++] = createStackEntry(first, a, limit, 0);
								   first = b;
							   }
							   else
							   {
								   last = a;
							   }
						   }
					   }
					   else
					   {
						   if (ssize == 0)
							   return;
						   struct StackEntry entry = stack[--ssize];
						   first = entry.a;
						   last = entry.b;
						   limit = entry.c;
					   }
				   }
			   }

			   void lsSort(global unsigned char *T, global int *SA, int ISA, int n, int depth) {
				   int ISAd;
				   int first, last, i;
				   int t, skip;

				   for (ISAd = ISA + depth; -n < SA[0]; ISAd += (ISAd - ISA))
				   {
					   first = 0;
					   skip = 0;
					   do
					   {
						   if ((t = SA[first]) < 0)
						   {
							   first -= t;
							   skip += t;
						   }
						   else
						   {
							   if (skip != 0)
							   {
								   SA[first + skip] = skip;
								   skip = 0;
							   }
							   last = SA[ISA + t] + 1;
							   lsIntroSort(T, SA, n, ISA, ISAd, ISA + n, first, last);
							   first = last;
						   }
					   } while (first < n);
					   if (skip != 0)
					   {
						   SA[first + skip] = skip;
					   }
					   if (n < (ISAd - ISA))
					   {
						   first = 0;
						   do
						   {
							   if ((t = SA[first]) < 0)
							   {
								   first -= t;
							   }
							   else
							   {
								   last = SA[ISA + t] + 1;
								   for (i = first; i < last; ++i)
								   {
									   SA[ISA + SA[i]] = i;
								   }
								   first = last;
							   }
						   } while (first < n);
						   break;
					   }
				   }
			   }

			   struct PartitionResult {
				   int first;
				   int last;
			   };

			   struct PartitionResult createPartitionResult(int a, int b) {
				   struct PartitionResult res = {a, b};
				   return res;
			   }

			   struct PartitionResult
				   trPartition(global unsigned char *T, global int *SA, int n, int ISA, int ISAd, int ISAn, int first, int last, int v) {
					   int a, b, c, d, e, f;
					   int t, s;
					   int x = 0;

					   for (b = first - 1; (++b < last) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[b])) == v);)
						   ;
					   if (((a = b) < last) && (x < v))
					   {
						   for (; (++b < last) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[b])) <= v);)
						   {
							   if (x == v)
							   {
								   swapElements(SA, b, SA, a);
								   ++a;
							   }
						   }
					   }
					   for (c = last; (b < --c) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[c])) == v);)
						   ;
					   if ((b < (d = c)) && (x > v))
					   {
						   for (; (b < --c) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[c])) >= v);)
						   {
							   if (x == v)
							   {
								   swapElements(SA, c, SA, d);
								   --d;
							   }
						   }
					   }
					   for (; b < c;)
					   {
						   swapElements(SA, b, SA, c);
						   for (; (++b < c) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[b])) <= v);)
						   {
							   if (x == v)
							   {
								   swapElements(SA, b, SA, a);
								   ++a;
							   }
						   }
						   for (; (b < --c) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[c])) >= v);)
						   {
							   if (x == v)
							   {
								   swapElements(SA, c, SA, d);
								   --d;
							   }
						   }
					   }

					   if (a <= d)
					   {
						   c = b - 1;
						   if ((s = a - first) > (t = b - a))
						   {
							   s = t;
						   }
						   for (e = first, f = b - s; 0 < s; --s, ++e, ++f)
						   {
							   swapElements(SA, e, SA, f);
						   }
						   if ((s = d - c) > (t = last - d - 1))
						   {
							   s = t;
						   }
						   for (e = b, f = last - s; 0 < s; --s, ++e, ++f)
						   {
							   swapElements(SA, e, SA, f);
						   }
						   first += (b - a);
						   last -= (d - c);
					   }

					   struct PartitionResult fResult = {first, last};
					   return fResult;
				   }

			   void trCopy(global unsigned char *T, global int *SA, int n, int ISA, int ISAn, int first, int a, int b, int last, int depth) {
				   int c, d, e;
				   int s, v;

				   v = b - 1;

				   for (c = first, d = a - 1; c <= d; ++c)
				   {
					   if ((s = SA[c] - depth) < 0)
					   {
						   s += ISAn - ISA;
					   }
					   if (SA[ISA + s] == v)
					   {
						   SA[++d] = s;
						   SA[ISA + s] = d;
					   }
				   }
				   for (c = last - 1, e = d + 1, d = b; e < d; --c)
				   {
					   if ((s = SA[c] - depth) < 0)
					   {
						   s += ISAn - ISA;
					   }
					   if (SA[ISA + s] == v)
					   {
						   SA[--d] = s;
						   SA[ISA + s] = d;
					   }
				   }
			   }

			   struct TRBudget {
				   int budget;
				   int chance;
			   };

			   struct TRBudget createTRBudget(int a, int b) {
				   struct TRBudget res = {a, b};
				   return res;
			   }

			   bool updateTRBudget(struct TRBudget * t, int size, int n) {
				   t->budget -= n;
				   if (t->budget <= 0)
				   {
					   if (--t->chance == 0)
					   {
						   return false;
					   }
					   t->budget += size;
				   }
				   return true;
			   }

			   void trIntroSort(global unsigned char *T, global int *SA, int n, int ISA, int ISAd, int ISAn, int first, int last, struct TRBudget budget, int size) {
				   struct StackEntry stack[STACK_SIZE] = {{0, 0, 0, 0}};

				   int a, b, c, d, e, f;
				   int s, t;
				   int v, x = 0;
				   int limit, next;
				   int ssize;

				   for (ssize = 0, limit = trLog(last - first);;)
				   {
					   if (limit < 0)
					   {
						   if (limit == -1)
						   {
							   if (!updateTRBudget(&budget, size, last - first))
								   break;
							   struct PartitionResult result = trPartition(T, SA, n, ISA, ISAd - 1, ISAn, first, last, last - 1);
							   a = result.first;
							   b = result.last;
							   if ((first < a) || (b < last))
							   {
								   if (a < last)
								   {
									   for (c = first, v = a - 1; c < a; ++c)
									   {
										   SA[ISA + SA[c]] = v;
									   }
								   }
								   if (b < last)
								   {
									   for (c = a, v = b - 1; c < b; ++c)
									   {
										   SA[ISA + SA[c]] = v;
									   }
								   }

								   stack[ssize++] = createStackEntry(0, a, b, 0);
								   stack[ssize++] = createStackEntry(ISAd - 1, first, last, -2);
								   if ((a - first) <= (last - b))
								   {
									   if (1 < (a - first))
									   {
										   stack[ssize++] = createStackEntry(ISAd, b, last, trLog(last - b));
										   last = a;
										   limit = trLog(a - first);
									   }
									   else if (1 < (last - b))
									   {
										   first = b;
										   limit = trLog(last - b);
									   }
									   else
									   {
										   if (ssize == 0)
											   return;
										   struct StackEntry entry = stack[--ssize];
										   ISAd = entry.a;
										   first = entry.b;
										   last = entry.c;
										   limit = entry.d;
									   }
								   }
								   else
								   {
									   if (1 < (last - b))
									   {
										   stack[ssize++] = createStackEntry(ISAd, first, a, trLog(a - first));
										   first = b;
										   limit = trLog(last - b);
									   }
									   else if (1 < (a - first))
									   {
										   last = a;
										   limit = trLog(a - first);
									   }
									   else
									   {
										   if (ssize == 0)
											   return;
										   struct StackEntry entry = stack[--ssize];
										   ISAd = entry.a;
										   first = entry.b;
										   last = entry.c;
										   limit = entry.d;
									   }
								   }
							   }
							   else
							   {
								   for (c = first; c < last; ++c)
								   {
									   SA[ISA + SA[c]] = c;
								   }
								   if (ssize == 0)
									   return;
								   struct StackEntry entry = stack[--ssize];
								   ISAd = entry.a;
								   first = entry.b;
								   last = entry.c;
								   limit = entry.d;
							   }
						   }
						   else if (limit == -2)
						   {
							   a = stack[--ssize].b;
							   b = stack[ssize].c;
							   trCopy(T, SA, n, ISA, ISAn, first, a, b, last, ISAd - ISA);
							   if (ssize == 0)
								   return;
							   struct StackEntry entry = stack[--ssize];
							   ISAd = entry.a;
							   first = entry.b;
							   last = entry.c;
							   limit = entry.d;
						   }
						   else
						   {
							   if (0 <= SA[first])
							   {
								   a = first;
								   do
								   {
									   SA[ISA + SA[a]] = a;
								   } while ((++a < last) && (0 <= SA[a]));
								   first = a;
							   }
							   if (first < last)
							   {
								   a = first;
								   do
								   {
									   SA[a] = ~SA[a];
								   } while (SA[++a] < 0);
								   next = (SA[ISA + SA[a]] != SA[ISAd + SA[a]]) ? trLog(a - first + 1) : -1;
								   if (++a < last)
								   {
									   for (b = first, v = a - 1; b < a; ++b)
									   {
										   SA[ISA + SA[b]] = v;
									   }
								   }

								   if ((a - first) <= (last - a))
								   {
									   stack[ssize++] = createStackEntry(ISAd, a, last, -3);
									   ISAd += 1;
									   last = a;
									   limit = next;
								   }
								   else
								   {
									   if (1 < (last - a))
									   {
										   stack[ssize++] = createStackEntry(ISAd + 1, first, a, next);
										   first = a;
										   limit = -3;
									   }
									   else
									   {
										   ISAd += 1;
										   last = a;
										   limit = next;
									   }
								   }
							   }
							   else
							   {
								   if (ssize == 0)
									   return;
								   struct StackEntry entry = stack[--ssize];
								   ISAd = entry.a;
								   first = entry.b;
								   last = entry.c;
								   limit = entry.d;
							   }
						   }
						   continue;
					   }

					   if ((last - first) <= INSERTIONSORT_THRESHOLD)
					   {
						   if (!updateTRBudget(&budget, size, last - first))
							   break;
						   trInsertionSort(T, SA, n, ISA, ISAd, ISAn, first, last);
						   limit = -3;
						   continue;
					   }

					   if (limit-- == 0)
					   {
						   if (!updateTRBudget(&budget, size, last - first))
							   break;
						   trHeapSort(T, SA, n, ISA, ISAd, ISAn, first, last - first);
						   for (a = last - 1; first < a; a = b)
						   {
							   for (
								   x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[a]), b = a - 1;
								   (first <= b) && (trGetC(T, SA, n, ISA, ISAd, ISAn, SA[b]) == x);
								   --b)
							   {
								   SA[b] = ~SA[b];
							   }
						   }
						   limit = -3;
						   continue;
					   }

					   a = trPivot(T, SA, n, ISA, ISAd, ISAn, first, last);

					   swapElements(SA, first, SA, a);
					   v = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[first]);
					   for (b = first; (++b < last) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[b])) == v);)
						   ;
					   if (((a = b) < last) && (x < v))
					   {
						   for (; (++b < last) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[b])) <= v);)
						   {
							   if (x == v)
							   {
								   swapElements(SA, b, SA, a);
								   ++a;
							   }
						   }
					   }
					   for (c = last; (b < --c) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[c])) == v);)
						   ;
					   if ((b < (d = c)) && (x > v))
					   {
						   for (; (b < --c) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[c])) >= v);)
						   {
							   if (x == v)
							   {
								   swapElements(SA, c, SA, d);
								   --d;
							   }
						   }
					   }
					   for (; b < c;)
					   {
						   swapElements(SA, b, SA, c);
						   for (; (++b < c) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[b])) <= v);)
						   {
							   if (x == v)
							   {
								   swapElements(SA, b, SA, a);
								   ++a;
							   }
						   }
						   for (; (b < --c) && ((x = trGetC(T, SA, n, ISA, ISAd, ISAn, SA[c])) >= v);)
						   {
							   if (x == v)
							   {
								   swapElements(SA, c, SA, d);
								   --d;
							   }
						   }
					   }

					   if (a <= d)
					   {
						   c = b - 1;

						   if ((s = a - first) > (t = b - a))
						   {
							   s = t;
						   }
						   for (e = first, f = b - s; 0 < s; --s, ++e, ++f)
						   {
							   swapElements(SA, e, SA, f);
						   }
						   if ((s = d - c) > (t = last - d - 1))
						   {
							   s = t;
						   }
						   for (e = b, f = last - s; 0 < s; --s, ++e, ++f)
						   {
							   swapElements(SA, e, SA, f);
						   }

						   a = first + (b - a);
						   b = last - (d - c);
						   next = (SA[ISA + SA[a]] != v) ? trLog(b - a) : -1;

						   for (c = first, v = a - 1; c < a; ++c)
						   {
							   SA[ISA + SA[c]] = v;
						   }
						   if (b < last)
						   {
							   for (c = a, v = b - 1; c < b; ++c)
							   {
								   SA[ISA + SA[c]] = v;
							   }
						   }

						   if ((a - first) <= (last - b))
						   {
							   if ((last - b) <= (b - a))
							   {
								   if (1 < (a - first))
								   {
									   stack[ssize++] = createStackEntry(ISAd + 1, a, b, next);
									   stack[ssize++] = createStackEntry(ISAd, b, last, limit);
									   last = a;
								   }
								   else if (1 < (last - b))
								   {
									   stack[ssize++] = createStackEntry(ISAd + 1, a, b, next);
									   first = b;
								   }
								   else if (1 < (b - a))
								   {
									   ISAd += 1;
									   first = a;
									   last = b;
									   limit = next;
								   }
								   else
								   {
									   if (ssize == 0)
										   return;
									   struct StackEntry entry = stack[--ssize];
									   ISAd = entry.a;
									   first = entry.b;
									   last = entry.c;
									   limit = entry.d;
								   }
							   }
							   else if ((a - first) <= (b - a))
							   {
								   if (1 < (a - first))
								   {
									   stack[ssize++] = createStackEntry(ISAd, b, last, limit);
									   stack[ssize++] = createStackEntry(ISAd + 1, a, b, next);
									   last = a;
								   }
								   else if (1 < (b - a))
								   {
									   stack[ssize++] = createStackEntry(ISAd, b, last, limit);
									   ISAd += 1;
									   first = a;
									   last = b;
									   limit = next;
								   }
								   else
								   {
									   first = b;
								   }
							   }
							   else
							   {
								   if (1 < (b - a))
								   {
									   stack[ssize++] = createStackEntry(ISAd, b, last, limit);
									   stack[ssize++] = createStackEntry(ISAd, first, a, limit);
									   ISAd += 1;
									   first = a;
									   last = b;
									   limit = next;
								   }
								   else
								   {
									   stack[ssize++] = createStackEntry(ISAd, b, last, limit);
									   last = a;
								   }
							   }
						   }
						   else
						   {
							   if ((a - first) <= (b - a))
							   {
								   if (1 < (last - b))
								   {
									   stack[ssize++] = createStackEntry(ISAd + 1, a, b, next);
									   stack[ssize++] = createStackEntry(ISAd, first, a, limit);
									   first = b;
								   }
								   else if (1 < (a - first))
								   {
									   stack[ssize++] = createStackEntry(ISAd + 1, a, b, next);
									   last = a;
								   }
								   else if (1 < (b - a))
								   {
									   ISAd += 1;
									   first = a;
									   last = b;
									   limit = next;
								   }
								   else
								   {
									   stack[ssize++] = createStackEntry(ISAd, first, last, limit);
								   }
							   }
							   else if ((last - b) <= (b - a))
							   {
								   if (1 < (last - b))
								   {
									   stack[ssize++] = createStackEntry(ISAd, first, a, limit);
									   stack[ssize++] = createStackEntry(ISAd + 1, a, b, next);
									   first = b;
								   }
								   else if (1 < (b - a))
								   {
									   stack[ssize++] = createStackEntry(ISAd, first, a, limit);
									   ISAd += 1;
									   first = a;
									   last = b;
									   limit = next;
								   }
								   else
								   {
									   last = a;
								   }
							   }
							   else
							   {
								   if (1 < (b - a))
								   {
									   stack[ssize++] = createStackEntry(ISAd, first, a, limit);
									   stack[ssize++] = createStackEntry(ISAd, b, last, limit);
									   ISAd += 1;
									   first = a;
									   last = b;
									   limit = next;
								   }
								   else
								   {
									   stack[ssize++] = createStackEntry(ISAd, first, a, limit);
									   first = b;
								   }
							   }
						   }
					   }
					   else
					   {
						   if (!updateTRBudget(&budget, size, last - first))
							   break; // BUGFIX : Added to prevent an infinite loop in the original code
						   limit += 1;
						   ISAd += 1;
					   }
				   }

				   for (s = 0; s < ssize; ++s)
				   {
					   if (stack[s].d == -3)
					   {
						   lsUpdateGroup(T, SA, n, ISA, stack[s].b, stack[s].c);
					   }
				   }
			   }) +
		   R(
			   void trSort(global unsigned char *T, global int *SA, int ISA, int n, int depth) {
				   int first = 0, last;
				   int t;

				   if (-n < SA[0])
				   {
					   struct TRBudget budget = createTRBudget(n, trLog(n) * 2 / 3 + 1);
					   do
					   {
						   if ((t = SA[first]) < 0)
						   {
							   first -= t;
						   }
						   else
						   {
							   last = SA[ISA + t] + 1;
							   if (1 < (last - first))
							   {
								   trIntroSort(T, SA, n, ISA, ISA + depth, ISA + n, first, last, budget, n);
								   if (budget.chance == 0)
								   {
									   if (0 < first)
									   {
										   SA[0] = -first;
									   }
									   lsSort(T, SA, ISA, n, depth);
									   break;
								   }
							   }
							   first = last;
						   }
					   } while (first < n);
				   }
			   }

			   int BUCKET_B(int c0, int c1) {
				   return (c1 << 8) | c0;
			   }

			   int BUCKET_BSTAR(int c0, int c1) {
				   return (c0 << 8) | c1;
			   }

			   int sortTypeBstar(global unsigned char *T, global int *SA, int n, global int *bucketA, global int *bucketB, global int *tempbuf) {
				   int PAb, ISAb, bufoffset;
				   int i, j, k, t, m, bufsize;
				   int c0, c1;
				   int flag;

				   for (i = 1, flag = 1; i < n; ++i)
				   {
					   if (T[i - 1] != T[i])
					   {
						   if ((T[i - 1] & 0xff) > (T[i] & 0xff))
						   {
							   flag = 0;
						   }
						   break;
					   }
				   }
				   i = n - 1;
				   m = n;

				   int ti, ti1, t0;
				   if (((ti = (T[i] & 0xff)) < (t0 = (T[0] & 0xff))) || ((T[i] == T[0]) && (flag != 0)))
				   {
					   if (flag == 0)
					   {
						   ++bucketB[BUCKET_BSTAR(ti, t0)];
						   SA[--m] = i;
					   }
					   else
					   {
						   ++bucketB[BUCKET_B(ti, t0)];
					   }
					   for (--i; (0 <= i) && ((ti = (T[i] & 0xff)) <= (ti1 = (T[i + 1] & 0xff))); --i)
					   {
						   ++bucketB[BUCKET_B(ti, ti1)];
					   }
				   }

				   for (; 0 <= i;)
				   {
					   do
					   {
						   ++bucketA[T[i] & 0xff];
					   } while ((0 <= --i) && ((T[i] & 0xff) >= (T[i + 1] & 0xff)));
					   if (0 <= i)
					   {
						   ++bucketB[BUCKET_BSTAR(T[i] & 0xff, T[i + 1] & 0xff)];
						   SA[--m] = i;
						   for (--i; (0 <= i) && ((ti = (T[i] & 0xff)) <= (ti1 = (T[i + 1] & 0xff))); --i)
						   {
							   ++bucketB[BUCKET_B(ti, ti1)];
						   }
					   }
				   }
				   m = n - m;
				   if (m == 0)
				   {
					   for (i = 0; i < n; ++i)
					   {
						   SA[i] = i;
					   }
					   return 0;
				   }

				   for (c0 = 0, i = -1, j = 0; c0 < 256; ++c0)
				   {
					   t = i + bucketA[c0];
					   bucketA[c0] = i + j;
					   i = t + bucketB[BUCKET_B(c0, c0)];
					   for (c1 = c0 + 1; c1 < 256; ++c1)
					   {
						   j += bucketB[BUCKET_BSTAR(c0, c1)];
						   bucketB[(c0 << 8) | c1] = j;
						   i += bucketB[BUCKET_B(c0, c1)];
					   }
				   }

				   PAb = n - m;
				   ISAb = m;
				   for (i = m - 2; 0 <= i; --i)
				   {
					   t = SA[PAb + i];
					   c0 = T[t] & 0xff;
					   c1 = T[t + 1] & 0xff;
					   SA[--bucketB[BUCKET_BSTAR(c0, c1)]] = i;
				   }
				   t = SA[PAb + m - 1];
				   c0 = T[t] & 0xff;
				   c1 = T[t + 1] & 0xff;
				   SA[--bucketB[BUCKET_BSTAR(c0, c1)]] = m - 1;

				   global int *buf = SA;
				   bufoffset = m;
				   bufsize = n - (2 * m);
				   if (bufsize <= 256)
				   {
					   buf = tempbuf;
					   bufoffset = 0;
					   bufsize = 256;
				   }

				   for (c0 = 255, j = m; 0 < j; --c0)
				   {
					   for (c1 = 255; c0 < c1; j = i, --c1)
					   {
						   i = bucketB[BUCKET_BSTAR(c0, c1)];
						   if (1 < (j - i))
						   {
							   subStringSort(T, SA, n, PAb, i, j, buf, bufoffset, bufsize, 2, SA[i] == (m - 1), n);
						   }
					   }
				   }

				   for (i = m - 1; 0 <= i; --i)
				   {
					   if (0 <= SA[i])
					   {
						   j = i;
						   do
						   {
							   SA[ISAb + SA[i]] = i;
						   } while ((0 <= --i) && (0 <= SA[i]));
						   SA[i + 1] = i - j;
						   if (i <= 0)
						   {
							   break;
						   }
					   }
					   j = i;
					   do
					   {
						   SA[ISAb + (SA[i] = ~SA[i])] = j;
					   } while (SA[--i] < 0);
					   SA[ISAb + SA[i]] = j;
				   }

				   trSort(T, SA, ISAb, m, 1);

				   i = n - 1;
				   j = m;
				   if (((T[i] & 0xff) < (T[0] & 0xff)) || ((T[i] == T[0]) && (flag != 0)))
				   {
					   if (flag == 0)
					   {
						   SA[SA[ISAb + --j]] = i;
					   }
					   for (--i; (0 <= i) && ((T[i] & 0xff) <= (T[i + 1] & 0xff)); --i)
						   ;
				   }
				   for (; 0 <= i;)
				   {
					   for (--i; (0 <= i) && ((T[i] & 0xff) >= (T[i + 1] & 0xff)); --i)
						   ;
					   if (0 <= i)
					   {
						   SA[SA[ISAb + --j]] = i;
						   for (--i; (0 <= i) && ((T[i] & 0xff) <= (T[i + 1] & 0xff)); --i)
							   ;
					   }
				   }

				   for (c0 = 255, i = n - 1, k = m - 1; 0 <= c0; --c0)
				   {
					   for (c1 = 255; c0 < c1; --c1)
					   {
						   t = i - bucketB[BUCKET_B(c0, c1)];
						   bucketB[BUCKET_B(c0, c1)] = i + 1;

						   for (i = t, j = bucketB[BUCKET_BSTAR(c0, c1)]; j <= k; --i, --k)
						   {
							   SA[i] = SA[k];
						   }
					   }
					   t = i - bucketB[BUCKET_B(c0, c0)];
					   bucketB[BUCKET_B(c0, c0)] = i + 1;
					   if (c0 < 255)
					   {
						   bucketB[BUCKET_BSTAR(c0, c0 + 1)] = t + 1;
					   }
					   i = bucketA[c0];
				   }

				   return m;
			   }

			   int constructBWT(global unsigned char *T, global int *SA, int n, global int *bucketA, global int *bucketB) {
				   int i, j, t = 0;
				   int s, s1;
				   int c0 = 0, c1, c2 = 0;
				   int orig = -1;

				   for (c1 = 254; 0 <= c1; --c1)
				   {
					   for (
						   i = bucketB[BUCKET_BSTAR(c1, c1 + 1)], j = bucketA[c1 + 1], t = 0, c2 = -1;
						   i <= j;
						   --j)
					   {
						   if (0 <= (s1 = s = SA[j]))
						   {
							   if (--s < 0)
							   {
								   s = n - 1;
							   }
							   if ((c0 = (T[s] & 0xff)) <= c1)
							   {
								   SA[j] = ~s1;
								   if ((0 < s) && ((T[s - 1] & 0xff) > c0))
								   {
									   s = ~s;
								   }
								   if (c2 == c0)
								   {
									   SA[--t] = s;
								   }
								   else
								   {
									   if (0 <= c2)
									   {
										   bucketB[BUCKET_B(c2, c1)] = t;
									   }
									   SA[t = bucketB[BUCKET_B(c2 = c0, c1)] - 1] = s;
								   }
							   }
						   }
						   else
						   {
							   SA[j] = ~s;
						   }
					   }
				   }

				   for (i = 0; i < n; ++i)
				   {
					   if (0 <= (s1 = s = SA[i]))
					   {
						   if (--s < 0)
						   {
							   s = n - 1;
						   }
						   if ((c0 = (T[s] & 0xff)) >= (T[s + 1] & 0xff))
						   {
							   if ((0 < s) && ((T[s - 1] & 0xff) < c0))
							   {
								   s = ~s;
							   }
							   if (c0 == c2)
							   {
								   SA[++t] = s;
							   }
							   else
							   {
								   if (c2 != -1) // BUGFIX: Original code can write to bucketA[-1]
									   bucketA[c2] = t;
								   SA[t = bucketA[c2 = c0] + 1] = s;
							   }
						   }
					   }
					   else
					   {
						   s1 = ~s1;
					   }

					   if (s1 == 0)
					   {
						   SA[i] = T[n - 1];
						   orig = i;
					   }
					   else
					   {
						   SA[i] = T[s1 - 1];
					   }
				   }

				   return orig;
			   }) +
		   R(
			   int DivSufSortBWT(global unsigned char *T, global int *SA, global int *bucketA, global int *bucketB, global int *tempbuf, int n) {
				   if (n == 0)
				   {
					   return 0;
				   }
				   else if (n == 1)
				   {
					   SA[0] = T[0];
					   return 0;
				   }

				   for (int i = 0; i < BUCKET_A_SIZE; ++i)
				   {
					   bucketA[i] = 0;
				   }
				   for (int i = 0; i < BUCKET_B_SIZE; ++i)
				   {
					   bucketB[i] = 0;
				   }

				   int m = sortTypeBstar(T, SA, n, bucketA, bucketB, tempbuf);
				   if (0 < m)
				   {
					   return constructBWT(T, SA, n, bucketA, bucketB);
				   }

				   return 0;
			   }

			   /* Write bits syntax */
			   void writeBoolean(global bool *bitBuffer, global size_t *bitCount, bool value) {
				   bitBuffer[(*bitCount)++] = value;
			   }

			   void writeUnary(global bool *bitBuffer, global size_t *bitCount, int value) {
				   while (value-- > 0)
				   {
					   writeBoolean(bitBuffer, bitCount, true);
				   }
				   writeBoolean(bitBuffer, bitCount, false);
			   }

			   void writeBits(global bool *bitBuffer, global size_t *bitCount, int count, int value) {
				   for (int bitMask = (1 << (count - 1)); bitMask; bitMask >>= 1)
				   {
					   writeBoolean(bitBuffer, bitCount, bitMask & value);
				   }
			   }

			   void writeInteger(global bool *bitBuffer, global size_t *bitCount, int value) {
				   writeBits(bitBuffer, bitCount, 16, (value >> 16) & 0xffff);
				   writeBits(bitBuffer, bitCount, 16, value & 0xffff);
			   }

			   void writeSymbolMap(global bool *bitBuffer, global size_t *bitCount, global bool *blockValuesPresent) {
				   bool condensedInUse[16] = {0};
				   for (int i = 0; i < 16; ++i)
				   {
					   for (int j = 0, k = i << 4; j < 16; ++j, ++k)
					   {
						   if (blockValuesPresent[k])
						   {
							   condensedInUse[i] = true;
						   }
					   }
				   }

				   for (int i = 0; i < 16; ++i)
				   {
					   writeBoolean(bitBuffer, bitCount, condensedInUse[i]);
				   }

				   for (int i = 0; i < 16; ++i)
				   {
					   if (condensedInUse[i])
					   {
						   for (int j = 0, k = i * 16; j < 16; ++j, ++k)
						   {
							   writeBoolean(bitBuffer, bitCount, blockValuesPresent[k]);
						   }
					   }
				   }
			   }

			   /* MTF */
			   int valueToFront(global int *mtf, int value) {
				   int index = 0;
				   int temp = mtf[0];

				   if (value == temp)
				   {
					   return index;
				   }

				   mtf[0] = value;
				   while (temp != value)
				   {
					   index++;
					   int swapTmp = mtf[index];
					   mtf[index] = temp;
					   temp = swapTmp;
				   }

				   return index;
			   }

			   int valueToFrontNonGlobal(int *mtf, int value) {
				   int index = 0;
				   int temp = mtf[0];

				   if (value == temp)
				   {
					   return index;
				   }

				   mtf[0] = value;
				   while (temp != value)
				   {
					   index++;
					   int swapTmp = mtf[index];
					   mtf[index] = temp;
					   temp = swapTmp;
				   }

				   return index;
			   }

			   struct MTFResult {
				   int mtfLength;
				   int alphabetSize;
			   };

			   struct MTFResult MTFAndRLE2StageEncoder(global int *bwtBlock, int bwtLength, global bool *bwtValuesInUse, global int *mtfSymbolFrequencies, global int *huffmanSymbolMap, global int *symbolMTF) {
				   int totalUniqueValues = 0;
				   for (int i = 0; i < ALPHABET_SIZE; i++)
				   {
					   huffmanSymbolMap[i] = 0;
					   symbolMTF[i] = i;
					   if (bwtValuesInUse[i])
					   {
						   huffmanSymbolMap[i] = totalUniqueValues++;
					   }
				   }

				   int endOfBlockSymbol = totalUniqueValues + 1;
				   int mtfIndex = 0;
				   int repeatCount = 0;
				   int totalRunAs = 0;
				   int totalRunBs = 0;
				   for (int i = 0; i < bwtLength; i++)
				   {
					   int mtfPosition = valueToFront(symbolMTF, huffmanSymbolMap[bwtBlock[i] & 0xff]);

					   if (mtfPosition == 0)
					   {
						   repeatCount++;
					   }
					   else
					   {
						   if (repeatCount > 0)
						   {
							   repeatCount--;
							   while (true)
							   {
								   if ((repeatCount & 1) == 0)
								   {
									   bwtBlock[mtfIndex++] = HUFFMAN_SYMBOL_RUNA;
									   totalRunAs++;
								   }
								   else
								   {
									   bwtBlock[mtfIndex++] = HUFFMAN_SYMBOL_RUNB;
									   totalRunBs++;
								   }

								   if (repeatCount <= 1)
								   {
									   break;
								   }
								   repeatCount = (repeatCount - 2) >> 1;
							   }
							   repeatCount = 0;
						   }
						   bwtBlock[mtfIndex++] = mtfPosition + 1;
						   mtfSymbolFrequencies[mtfPosition + 1]++;
					   }
				   }
				   if (repeatCount > 0)
				   {
					   repeatCount--;
					   while (true)
					   {
						   if ((repeatCount & 1) == 0)
						   {
							   bwtBlock[mtfIndex++] = HUFFMAN_SYMBOL_RUNA;
							   totalRunAs++;
						   }
						   else
						   {
							   bwtBlock[mtfIndex++] = HUFFMAN_SYMBOL_RUNB;
							   totalRunBs++;
						   }

						   if (repeatCount <= 1)
						   {
							   break;
						   }
						   repeatCount = (repeatCount - 2) >> 1;
					   }
				   }

				   bwtBlock[mtfIndex] = endOfBlockSymbol;
				   mtfSymbolFrequencies[endOfBlockSymbol]++;
				   mtfSymbolFrequencies[HUFFMAN_SYMBOL_RUNA] += totalRunAs;
				   mtfSymbolFrequencies[HUFFMAN_SYMBOL_RUNB] += totalRunBs;

				   int mtfLength = mtfIndex + 1;
				   int alphabetSize = endOfBlockSymbol + 1;
				   struct MTFResult res = {mtfLength, alphabetSize};
				   return res;
			   }

			   /* HUFFMAN */
			   int SignificantBits(int x) {
				   int n;
				   for (n = 0; x > 0; n++)
				   {
					   x >>= 1;
				   }
				   return n;
			   }

			   int first(int *array, int arraySize, int i, int nodesToMove) {
				   const int length = arraySize;
				   const int limit = i;
				   int k = length - 2;

				   while ((i >= nodesToMove) && ((array[i] % length) > limit))
				   {
					   k = i;
					   i -= (limit - i + 1);
				   }
				   i = nodesToMove - 1 > i ? nodesToMove - 1 : i;

				   while (k > (i + 1))
				   {
					   int temp = (i + k) >> 1;
					   if ((array[temp] % length) > limit)
					   {
						   k = temp;
					   }
					   else
					   {
						   i = temp;
					   }
				   }

				   return k;
			   }

			   void setExtendedParentPointers(int *array, int arraySize) {
				   int length = arraySize;

				   array[0] += array[1];

				   for (int headNode = 0, tailNode = 1, topNode = 2; tailNode < (length - 1); tailNode++)
				   {
					   int temp;
					   if ((topNode >= length) || (array[headNode] < array[topNode]))
					   {
						   temp = array[headNode];
						   array[headNode++] = tailNode;
					   }
					   else
					   {
						   temp = array[topNode++];
					   }

					   if ((topNode >= length) || ((headNode < tailNode) && (array[headNode] < array[topNode])))
					   {
						   temp += array[headNode];
						   array[headNode++] = tailNode + length;
					   }
					   else
					   {
						   temp += array[topNode++];
					   }

					   array[tailNode] = temp;
				   }
			   }

			   int findNodesToRelocate(int *array, int arraySize, int maximumLength) {
				   int currentNode = arraySize - 2;
				   for (int currentDepth = 1; (currentDepth < (maximumLength - 1)) && (currentNode > 1); currentDepth++)
				   {
					   currentNode = first(array, arraySize, currentNode - 1, 0);
				   }

				   return currentNode;
			   }

			   void allocateNodeLengths(int *array, int arraySize) {
				   int firstNode = arraySize - 2;
				   int nextNode = arraySize - 1;

				   for (int currentDepth = 1, availableNodes = 2; availableNodes > 0; currentDepth++)
				   {
					   int lastNode = firstNode;
					   firstNode = first(array, arraySize, lastNode - 1, 0);

					   for (int i = availableNodes - (lastNode - firstNode); i > 0; i--)
					   {
						   array[nextNode--] = currentDepth;
					   }

					   availableNodes = (lastNode - firstNode) << 1;
				   }
			   }

			   void allocateNodeLengthsWithRelocation(int *array, int arraySize, int nodesToMove, int insertDepth) {
				   int firstNode = arraySize - 2;
				   int nextNode = arraySize - 1;
				   int currentDepth = (insertDepth == 1) ? 2 : 1;
				   int nodesLeftToMove = (insertDepth == 1) ? nodesToMove - 2 : nodesToMove;

				   for (int availableNodes = currentDepth << 1; availableNodes > 0; currentDepth++)
				   {
					   int lastNode = firstNode;
					   firstNode = (firstNode <= nodesToMove) ? firstNode : first(array, arraySize, lastNode - 1, nodesToMove);

					   int offset = 0;
					   if (currentDepth >= insertDepth)
					   {
						   offset = (nodesLeftToMove < (1 << (currentDepth - insertDepth))) ? (nodesLeftToMove) : (1 << (currentDepth - insertDepth));
					   }
					   else if (currentDepth == (insertDepth - 1))
					   {
						   offset = 1;
						   if (array[firstNode] == lastNode)
						   {
							   firstNode++;
						   }
					   }

					   for (int i = availableNodes - (lastNode - firstNode + offset); i > 0; i--)
					   {
						   array[nextNode--] = currentDepth;
					   }

					   nodesLeftToMove -= offset;
					   availableNodes = (lastNode - firstNode + offset) << 1;
				   }
			   }

			   void allocateHuffmanCodeLengths(int *array, int arraySize, int maximumLength) {
				   switch (arraySize)
				   {
				   case 2:
					   array[1] = 1;
				   case 1:
					   array[0] = 1;
					   return;
				   }

				   setExtendedParentPointers(array, arraySize);
				   int nodesToRelocate = findNodesToRelocate(array, arraySize, maximumLength);

				   if ((array[0] % arraySize) >= nodesToRelocate)
				   {
					   allocateNodeLengths(array, arraySize);
				   }
				   else
				   {
					   int insertDepth = maximumLength - SignificantBits(nodesToRelocate - 1);
					   allocateNodeLengthsWithRelocation(array, arraySize, nodesToRelocate, insertDepth);
				   }
			   }) +
		   R(
			   int selectTableCount(int mtfLength) {
				   if (mtfLength >= 2400)
					   return 6;
				   if (mtfLength >= 1200)
					   return 5;
				   if (mtfLength >= 600)
					   return 4;
				   if (mtfLength >= 200)
					   return 3;
				   return 2;
			   }

			   void sortArr(int *a, int size) {
				   for (int i = 0; i < size; ++i)
				   {
					   for (int j = i + 1; j < size; ++j)
					   {
						   if (a[i] > a[j])
						   {
							   int temp = a[i];
							   a[i] = a[j];
							   a[j] = temp;
						   }
					   }
				   }
			   }

			   void generateHuffmanCodeLengths(int alphabetSize, int *symbolFrequencies, int *codeLengths) {
				   int mergedFrequenciesAndIndices[HUFFMAN_MAXIMUM_ALPHABET_SIZE];
				   int sortedFrequencies[HUFFMAN_MAXIMUM_ALPHABET_SIZE];

				   for (int i = 0; i < alphabetSize; i++)
				   {
					   mergedFrequenciesAndIndices[i] = (symbolFrequencies[i] << 9) | i;
				   }

				   sortArr(mergedFrequenciesAndIndices, alphabetSize);

				   for (int i = 0; i < alphabetSize; i++)
				   {
					   sortedFrequencies[i] = mergedFrequenciesAndIndices[i] >> 9;
				   }

				   allocateHuffmanCodeLengths(sortedFrequencies, alphabetSize, HUFFMAN_ENCODE_MAXIMUM_CODE_LENGTH);

				   for (int i = 0; i < alphabetSize; i++)
				   {
					   codeLengths[mergedFrequenciesAndIndices[i] & 0x1ff] = sortedFrequencies[i];
				   }
			   }

			   void generateHuffmanOptimisationSeeds(int mtfLength,
													 int mtfAlphabetSize,
													 global int *mtfSymbolFrequencies,
													 int huffmanCodeLengths[HUFFMAN_MAXIMUM_TABLES][HUFFMAN_MAXIMUM_ALPHABET_SIZE],
													 int totalTables) {
				   int remainingLength = mtfLength;
				   int lowCostEnd = -1;

				   for (int i = 0; i < totalTables; i++)
				   {
					   int targetCumulativeFrequency = remainingLength / (totalTables - i);
					   int lowCostStart = lowCostEnd + 1;
					   int actualCumulativeFrequency = 0;

					   while ((actualCumulativeFrequency < targetCumulativeFrequency) && (lowCostEnd < (mtfAlphabetSize - 1)))
					   {
						   actualCumulativeFrequency += mtfSymbolFrequencies[++lowCostEnd];
					   }

					   if ((lowCostEnd > lowCostStart) && (i != 0) && (i != (totalTables - 1)) && ((totalTables - i) % 2) == 0)
					   {
						   actualCumulativeFrequency -= mtfSymbolFrequencies[lowCostEnd--];
					   }

					   for (int j = 0; j < mtfAlphabetSize; j++)
					   {
						   if ((j < lowCostStart) || (j > lowCostEnd))
						   {
							   huffmanCodeLengths[i][j] = HUFFMAN_HIGH_SYMBOL_COST;
						   }
					   }

					   remainingLength -= actualCumulativeFrequency;
				   }
			   }

			   void optimiseSelectorsAndHuffmanTables(global int *mtfBlock,
													  int mtfLength,
													  int mtfAlphabetSize,
													  int huffmanCodeLengths[HUFFMAN_MAXIMUM_TABLES][HUFFMAN_MAXIMUM_ALPHABET_SIZE],
													  int totalTables,
													  global int *selectors,
													  bool storeSelectors) {
				   int tableFrequencies[HUFFMAN_MAXIMUM_TABLES][HUFFMAN_MAXIMUM_ALPHABET_SIZE];
				   int cost[HUFFMAN_MAXIMUM_TABLES];

				   int selectorIndex = 0;
				   for (int groupStart = 0; groupStart < mtfLength;)
				   {
					   int groupEnd = (groupStart + HUFFMAN_GROUP_RUN_LENGTH < mtfLength ? groupStart + HUFFMAN_GROUP_RUN_LENGTH : mtfLength) - 1;

					   for (int i = 0; i < totalTables; ++i)
					   {
						   cost[i] = 0;
					   }
					   for (int i = groupStart; i <= groupEnd; i++)
					   {
						   int value = mtfBlock[i];
						   for (int j = 0; j < totalTables; j++)
						   {
							   cost[j] += huffmanCodeLengths[j][value];
						   }
					   }

					   int bestTable = 0;
					   int bestCost = cost[0];
					   for (int i = 1; i < totalTables; i++)
					   {
						   if (cost[i] < bestCost)
						   {
							   bestCost = cost[i];
							   bestTable = i;
						   }
					   }

					   for (int i = groupStart; i <= groupEnd; i++)
					   {
						   tableFrequencies[bestTable][mtfBlock[i]]++;
					   }

					   if (storeSelectors)
					   {
						   selectors[selectorIndex++] = bestTable;
					   }

					   groupStart = groupEnd + 1;
				   }

				   for (int i = 0; i < totalTables; i++)
				   {
					   generateHuffmanCodeLengths(mtfAlphabetSize, tableFrequencies[i], huffmanCodeLengths[i]);
				   }
			   }

			   void assignHuffmanCodeSymbols(int mtfAlphabetSize,
											 int huffmanCodeLengths[HUFFMAN_MAXIMUM_TABLES][HUFFMAN_MAXIMUM_ALPHABET_SIZE],
											 int huffmanMergedCodeSymbols[HUFFMAN_MAXIMUM_TABLES][HUFFMAN_MAXIMUM_ALPHABET_SIZE],
											 int totalTables) {
				   for (int i = 0; i < totalTables; i++)
				   {
					   int minimumLength = 32;
					   int maximumLength = 0;

					   for (int j = 0; j < mtfAlphabetSize; ++j)
					   {
						   int length = huffmanCodeLengths[i][j];
						   if (length > maximumLength)
						   {
							   maximumLength = length;
						   }
						   if (length < minimumLength)
						   {
							   minimumLength = length;
						   }
					   }

					   int code = 0;
					   for (int j = minimumLength; j <= maximumLength; j++)
					   {
						   for (int k = 0; k < mtfAlphabetSize; k++)
						   {
							   if ((huffmanCodeLengths[i][k] & 0xff) == j)
							   {
								   huffmanMergedCodeSymbols[i][k] = (j << 24) | code;
								   code++;
							   }
						   }
						   code <<= 1;
					   }
				   }
			   }

			   void writeSelectorsAndHuffmanTables(global bool *bitBuffer,
												   global size_t *bitCount,
												   global int *selectors,
												   int selectorsSize,
												   int huffmanCodeLengths[HUFFMAN_MAXIMUM_TABLES][HUFFMAN_MAXIMUM_ALPHABET_SIZE],
												   int huffmanCodeLengthsSize,
												   int mtfAlphabetSize) {
				   int totalTables = huffmanCodeLengthsSize;
				   int totalSelectors = selectorsSize;

				   writeBits(bitBuffer, bitCount, 3, totalTables);
				   writeBits(bitBuffer, bitCount, 15, totalSelectors);

				   int symbolMTF[ALPHABET_SIZE] = {0};
				   for (int i = 0; i < ALPHABET_SIZE; ++i)
				   {
					   symbolMTF[i] = i;
				   }
				   for (int i = 0; i < totalSelectors; i++)
				   {
					   writeUnary(bitBuffer, bitCount, valueToFrontNonGlobal(symbolMTF, selectors[i]));
				   }

				   // Write the Huffman tables
				   for (int i = 0; i < huffmanCodeLengthsSize; ++i)
				   {
					   int *tableLengths = huffmanCodeLengths[i];
					   int currentLength = tableLengths[0];

					   writeBits(bitBuffer, bitCount, 5, currentLength);

					   for (int j = 0; j < mtfAlphabetSize; j++)
					   {
						   int codeLength = tableLengths[j];
						   int value = (currentLength < codeLength) ? 2 : 3;

						   int delta = codeLength - currentLength;
						   if (delta < 0)
						   {
							   delta *= -1;
						   }

						   while (delta-- > 0)
						   {
							   writeBits(bitBuffer, bitCount, 2, value);
						   }
						   writeBoolean(bitBuffer, bitCount, false);
						   currentLength = codeLength;
					   }
				   }
			   }

			   void writeBlockData(global bool *bitBuffer,
								   global size_t *bitCount,
								   global int *mtfBlock,
								   int mtfLength,
								   global int *selectors,
								   int huffmanMergedCodeSymbols[HUFFMAN_MAXIMUM_TABLES][HUFFMAN_MAXIMUM_ALPHABET_SIZE]) {
				   int selectorIndex = 0;
				   int mtfIndex = 0;
				   while (mtfIndex < mtfLength)
				   {
					   int groupEnd = (mtfIndex + HUFFMAN_GROUP_RUN_LENGTH < mtfLength ? mtfIndex + HUFFMAN_GROUP_RUN_LENGTH : mtfLength) - 1;
					   int *tableMergedCodeSymbols = huffmanMergedCodeSymbols[selectors[selectorIndex++]];

					   while (mtfIndex <= groupEnd)
					   {
						   int mergedCodeSymbol = tableMergedCodeSymbols[mtfBlock[mtfIndex++]];
						   writeBits(bitBuffer, bitCount, mergedCodeSymbol >> 24, mergedCodeSymbol);
					   }
				   }
			   }

			   void HuffmanStageEncoder(global bool *bitBuffer,
										global size_t *bitCount,
										global int *mtfBlock,
										int mtfLength,
										int mtfAlphabetSize,
										global int *mtfSymbolFrequencies,
										global int *selectors) {
				   int totalTables = selectTableCount(mtfLength);
				   int huffmanCodeLengths[HUFFMAN_MAXIMUM_TABLES][HUFFMAN_MAXIMUM_ALPHABET_SIZE] = {0};
				   int huffmanMergedCodeSymbols[HUFFMAN_MAXIMUM_TABLES][HUFFMAN_MAXIMUM_ALPHABET_SIZE] = {0};
				   int selectorsSize = (mtfLength + HUFFMAN_GROUP_RUN_LENGTH - 1) / HUFFMAN_GROUP_RUN_LENGTH;

				   generateHuffmanOptimisationSeeds(mtfLength,
													mtfAlphabetSize,
													mtfSymbolFrequencies,
													huffmanCodeLengths,
													totalTables);

				   for (int i = 3; i >= 0; i--)
				   {
					   optimiseSelectorsAndHuffmanTables(mtfBlock,
														 mtfLength,
														 mtfAlphabetSize,
														 huffmanCodeLengths,
														 totalTables,
														 selectors,
														 i == 0);
				   }
				   assignHuffmanCodeSymbols(mtfAlphabetSize, huffmanCodeLengths, huffmanMergedCodeSymbols, totalTables);

				   writeSelectorsAndHuffmanTables(bitBuffer, bitCount, selectors, selectorsSize, huffmanCodeLengths, totalTables, mtfAlphabetSize);
				   writeBlockData(bitBuffer, bitCount, mtfBlock, mtfLength, selectors, huffmanMergedCodeSymbols);
			   }

			   /* Run MTF, RLE2, HUFFMAN */
			   void close_block(global unsigned char *preBWTblock,
								global int *block,
								int blockLength,
								global int *bucketA,
								global int *bucketB,
								global int *bwtTempBuff,
								global bool *bitBuffer,
								global size_t *bitCount,
								global bool *blockValuesPresent,
								global int *mtfSymbolFrequencies,
								global int *huffmanSymbolMap,
								global int *symbolMTF,
								global int *selectors) {
				   // Wrap for BWT
				   preBWTblock[blockLength] = preBWTblock[0];
				   int bwtStartPointer = DivSufSortBWT(preBWTblock, block, bucketA, bucketB, bwtTempBuff, blockLength);

				   writeBits(bitBuffer, bitCount, 24, bwtStartPointer);

				   writeSymbolMap(bitBuffer, bitCount, blockValuesPresent);
				   struct MTFResult mtfEncoder = MTFAndRLE2StageEncoder(block, blockLength, blockValuesPresent, mtfSymbolFrequencies, huffmanSymbolMap, symbolMTF);

				   HuffmanStageEncoder(bitBuffer, bitCount, block, mtfEncoder.mtfLength, mtfEncoder.alphabetSize, mtfSymbolFrequencies, selectors);
			   }

			   kernel void kernel_close(global bool *isEmptyCompressor,
										global unsigned char *blocks,
										global int *bwtBlocks,
										global size_t *blockLengths,
										global int *bucketsA,
										global int *bucketsB,
										global int *bwtTempBuffs,
										global bool *bitOutBuffers,
										global size_t *bitOutCnts,
										global bool *blocksValuePresent,
										global int *mtfsSymbolFrequencies,
										global int *huffmanSymbolMaps,
										global int *symbolMTFs,
										global int *huffmanSelectors,
										private const int blockCnt,
										private const unsigned int streamBlockSize) {
				   const uint i = get_global_id(0);
				   if (i >= blockCnt || isEmptyCompressor[i])
				   {
					   return;
				   }

				   close_block(blocks + i * streamBlockSize,
							   bwtBlocks + i * streamBlockSize,
							   blockLengths[i],
							   bucketsA + i * BUCKET_A_SIZE,
							   bucketsB + i * BUCKET_B_SIZE,
							   bwtTempBuffs + i * ALPHABET_SIZE,
							   bitOutBuffers + i * 16 * streamBlockSize,
							   &(bitOutCnts[i]),
							   blocksValuePresent + i * ALPHABET_SIZE,
							   mtfsSymbolFrequencies + i * ALPHABET_SIZE,
							   huffmanSymbolMaps + i * ALPHABET_SIZE,
							   symbolMTFs + i * ALPHABET_SIZE,
							   huffmanSelectors + i * (streamBlockSize + HUFFMAN_GROUP_RUN_LENGTH - 1) / HUFFMAN_GROUP_RUN_LENGTH);
			   }

		   );
} // ############################################################### end of OpenCL C code #####################################################################