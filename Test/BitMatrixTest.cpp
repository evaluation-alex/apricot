// (C) 2016 University of Bristol. See LICENSE.txt

#include <sys/time.h>
#include <iostream>
#include <smmintrin.h>

#include "OT/BitMatrix.h"
#include "Tools/random.h"
#include "Tools/time-func.h"

int main()
{
    PRNG G;
    G.ReSeed();

    square128 square;
    memset(&square, 0, sizeof(square));
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            square.bytes[i][j] = j * 16 + i;
    square128 tmp;
    tmp = square;
    square.transpose();
    square.check_transpose(tmp);
    square.randomize(G);
    tmp = square;
    square.transpose();
    square.check_transpose(tmp);

    BitMatrix bm(128000);
    bm.randomize(G);
    timeval start, stop;
    gettimeofday(&start, NULL);
    for (int i = 0; i < 100; i++)
        bm.transpose();
    gettimeofday(&stop, NULL);
    cout << timeval_diff(&start, &stop) << endl;
    BitMatrix dual(bm);
    bm.transpose();
    bm.check_transpose(dual);

    bm.randomize(G);
    BitMatrix copy(bm);
    BitMatrix zero(128000);
    copy ^= bm;
    if (copy != zero)
        cout << "Error with XOR";

    bm.randomize(G);
    octetStream os;
    copy = bm;
    bm.pack(os);
    bm.unpack(os);
    if (bm != copy)
        cout << "Error with packing";

    BitVector bv;
    bv.resize(128);
    bv.randomize(G);
    square128 a, b;
    a.randomize(G);
    b = a;
    a ^= bv;
    b ^= a;
    for (int i = 0; i < 128; i++)
        a.rows[i] = _mm_loadu_si128((__m128i*)bv.get_ptr());
    if (!(a == b))
        cout << "Error with vector XOR";

    bv.randomize(G);
    a.set_zero();
    b.randomize(G);
    a.conditional_xor(bv, b);
    square128 c = a;
    c ^= b;
    for (int i = 0; i < 128; i++)
        if (bv.get_bit(i))
        {
            if (!_mm_test_all_zeros(c.rows[i], c.rows[i]))
                cout << "Error with conditional XOR, case 1";
        }
        else
        {
            if (!_mm_test_all_zeros(a.rows[i], c.rows[i]))
                cout << "Error with conditional XOR, case 0";
        }
}

