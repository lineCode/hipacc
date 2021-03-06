//
// Copyright (c) 2020, University of Erlangen-Nuremberg
// Copyright (c) 2012, University of Erlangen-Nuremberg
// Copyright (c) 2012, Siemens AG
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "hipacc.hpp"

#include <iostream>
#include <hipacc_helper.hpp>

#define WIDTH 512
#define HEIGHT 512
#define SIZE_X 5
#define SIZE_Y 5
#define TYPE uchar

using namespace hipacc;
using namespace hipacc::math;

// Kernel description in Hipacc
class PointOperatorExample : public Kernel<TYPE> {
    private:
        Accessor<TYPE> &in;

    public:
        PointOperatorExample(IterationSpace<TYPE> &iter, Accessor<TYPE> &acc)
              : Kernel(iter), in(acc) {
            add_accessor(&in);
        }

        void kernel() {
            TYPE interm_pixel = in();
            interm_pixel += 3;
            output() = interm_pixel;
        }
};

class LocalOperatorExample : public Kernel<TYPE> {
  private:
    Accessor<TYPE> &Input;
    Mask<float> &mask;

  public:
    LocalOperatorExample(IterationSpace<TYPE> &IS, Accessor<TYPE> &Input,
             Mask<float> &mask)
          : Kernel(IS), Input(Input), mask(mask) {
        add_accessor(&Input);
    }

    void kernel() {
        output() = (TYPE)(convolve(mask, Reduce::SUM, [&] () -> float {
                return mask() * Input(mask);
            }) + 0.5f);
    }
};

// forward declaration of reference implementation
void kernel_fusion(TYPE *in, TYPE *out, float *filter,
                   int size_x, int size_y, int width, int height);

/*************************************************************************
 * Main function                                                         *
 *************************************************************************/
HIPACC_CODEGEN int main(int argc, const char **argv) {
    const int width = WIDTH;
    const int height = HEIGHT;
    const int size_x = SIZE_X;
    const int size_y = SIZE_Y;
    const int offset_x = size_x >> 1;
    const int offset_y = size_y >> 1;

    // convolution filter mask
    const float coef[SIZE_Y][SIZE_X] = {
        { 0.005008f, 0.017300f, 0.026151f, 0.017300f, 0.005008f },
        { 0.017300f, 0.059761f, 0.090339f, 0.059761f, 0.017300f },
        { 0.026151f, 0.090339f, 0.136565f, 0.090339f, 0.026151f },
        { 0.017300f, 0.059761f, 0.090339f, 0.059761f, 0.017300f },
        { 0.005008f, 0.017300f, 0.026151f, 0.017300f, 0.005008f }
    };

    // host memory for image of width x height pixels, random
    TYPE *input = (TYPE*)load_data<TYPE>(width, height);
    TYPE *ref_out = new TYPE[width*height];

    std::cout << "Testing Hipacc kernel fusion ..." << std::endl;

    //************************************************************************//

    // input and output image of width x height pixels
    Image<TYPE> in(width, height, input);
    Image<TYPE> out(width, height);

    // test mixed operators kernel fusion
    // p -> p -> l -> p -> p -> l -> p

    Accessor<TYPE> acc0(in);
    Image<TYPE> buf0(width, height);
    IterationSpace<TYPE> iter0(buf0);
    PointOperatorExample pointOp0(iter0, acc0);

    Accessor<TYPE> acc1(buf0);
    Image<TYPE> buf1(width, height);
    IterationSpace<TYPE> iter1(buf1);
    PointOperatorExample pointOp1(iter1, acc1);

    Mask<float> mask(coef);
    Image<TYPE> buf2(width, height);
    BoundaryCondition<TYPE> bound2(buf1, mask, Boundary::CLAMP);
    Accessor<TYPE> acc2(bound2);
    IterationSpace<TYPE> iter2(buf2);
    LocalOperatorExample localOp2(iter2, acc2, mask);

    Accessor<TYPE> acc3(buf2);
    Image<TYPE> buf3(width, height);
    IterationSpace<TYPE> iter3(buf3);
    PointOperatorExample pointOp3(iter3, acc3);

    Accessor<TYPE> acc4(buf3);
    Image<TYPE> buf4(width, height);
    IterationSpace<TYPE> iter4(buf4);
    PointOperatorExample pointOp4(iter4, acc4);

    Image<TYPE> buf5(width, height);
    BoundaryCondition<TYPE> bound5(buf4, mask, Boundary::CLAMP);
    Accessor<TYPE> acc5(bound5);
    IterationSpace<TYPE> iter5(buf5);
    LocalOperatorExample localOp5(iter5, acc5, mask);

    Accessor<TYPE> acc6(buf5);
    Image<TYPE> buf6(width, height);
    IterationSpace<TYPE> iter6(buf6);
    PointOperatorExample pointOp6(iter6, acc6);

    Accessor<TYPE> acc7(buf6);
    IterationSpace<TYPE> iter7(out);
    PointOperatorExample pointOp7(iter7, acc7);

    // execution after all decls
    pointOp0.execute();
    pointOp1.execute();
    localOp2.execute();
    pointOp3.execute();
    pointOp4.execute();
    localOp5.execute();
    pointOp6.execute();
    pointOp7.execute();

    // get pointer to result data
    TYPE *output = out.data();

    //************************************************************************//
    std::cout << "Calculating reference ..." << std::endl;
    kernel_fusion(input, ref_out, (float*)coef, size_x, size_y, width, height);
    compare_results(output, ref_out, width, height, offset_x*2, offset_y*2);

    // free memory
    delete[] input;
    delete[] ref_out;

    return EXIT_SUCCESS;
}

// kernel fusion reference
void point_kernel(TYPE *in, TYPE *out, int width, int height) {
    for (int p = 0; p < width*height; ++p) {
        TYPE interm_pixel = in[p];
        interm_pixel += 3;
        out[p] = interm_pixel;
    }
}

void local_kernel(TYPE *in, TYPE *out, float *filter,
                     int size_x, int size_y, int width, int height) {
    int anchor_x = size_x >> 1;
    int anchor_y = size_y >> 1;
    int upper_x = width  - anchor_x;
    int upper_y = height - anchor_y;

    for (int y=anchor_y; y<upper_y; ++y) {
        for (int x=anchor_x; x<upper_x; ++x) {
            float sum = 0.5f;
            for (int yf = -anchor_y; yf<=anchor_y; ++yf) {
                for (int xf = -anchor_x; xf<=anchor_x; ++xf) {
                    sum += filter[(yf+anchor_y)*size_x + xf+anchor_x]
                            * in[(y+yf)*width + x + xf];
                }
            }
            out[y*width + x] = (TYPE) (sum);
        }
    }
}

void kernel_fusion(TYPE *in, TYPE *out, float *filter,
                   int size_x, int size_y, int width, int height) {
  TYPE *ref_buf0 = new TYPE[width*height];
  TYPE *ref_buf1 = new TYPE[width*height];
  TYPE *ref_buf2 = new TYPE[width*height];
  TYPE *ref_buf3 = new TYPE[width*height];
  TYPE *ref_buf4 = new TYPE[width*height];
  TYPE *ref_buf5 = new TYPE[width*height];
  TYPE *ref_buf6 = new TYPE[width*height];

  point_kernel(in, ref_buf0, width, height);
  point_kernel(ref_buf0, ref_buf1, width, height);
  local_kernel(ref_buf1, ref_buf2, filter, size_x, size_y, width, height);
  point_kernel(ref_buf2, ref_buf3, width, height);
  point_kernel(ref_buf3, ref_buf4, width, height);
  local_kernel(ref_buf4, ref_buf5, filter, size_x, size_y, width, height);
  point_kernel(ref_buf5, ref_buf6, width, height);
  point_kernel(ref_buf6, out, width, height);

  delete[] ref_buf0;
  delete[] ref_buf1;
  delete[] ref_buf2;
  delete[] ref_buf3;
  delete[] ref_buf4;
  delete[] ref_buf5;
  delete[] ref_buf6;
}
