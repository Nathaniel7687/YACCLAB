// Copyright(c) 2016 - 2017 Costantino Grana, Federico Bolelli, Lorenzo Baraldi and Roberto Vezzani
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
// *Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.
//
// * Neither the name of YACCLAB nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "labelingWu2009.h"

using namespace std;
using namespace cv;

REGISTER_LABELING(SAUF);
REGISTER_LABELING(SAUF2);

void SAUF::AllocateMemory()
{
    const size_t Plength = upperBound8Conn;
    P = (unsigned *)fastMalloc(sizeof(unsigned) * Plength); //array P for equivalences resolution
    return;
}

void SAUF::DeallocateMemory()
{
    fastFree(P);
    return;
}

unsigned SAUF::FirstScan()
{
    const int h = aImg.rows;
    const int w = aImg.cols;

    aImgLabels = Mat1i(aImg.size(), 0);
    P[0] = 0;	//first label is for background pixels
    unsigned lunique = 1;

    // Rosenfeld Mask
    // +-+-+-+
    // |p|q|r|
    // +-+-+-+
    // |s|x|
    // +-+-+

    // first scan
    for (int r = 0; r < h; ++r) {
        // Get row pointers
        uchar const * const img_row = aImg.ptr<uchar>(r);
        uchar const * const img_row_prev = (uchar *)(((char *)img_row) - aImg.step.p[0]);
        unsigned * const  imgLabels_row = aImgLabels.ptr<unsigned>(r);
        unsigned * const  imgLabels_row_prev = (unsigned *)(((char *)imgLabels_row) - aImgLabels.step.p[0]);

        for (int c = 0; c < w; ++c) {
        #define condition_p c>0 && r>0 && img_row_prev[c - 1]>0
        #define condition_q r>0 && img_row_prev[c]>0
        #define condition_r c < w - 1 && r > 0 && img_row_prev[c + 1] > 0
        #define condition_s c > 0 && img_row[c - 1] > 0
        #define condition_x img_row[c] > 0

            if (condition_x) {
                if (condition_q) {
                    //x <- q
                    imgLabels_row[c] = imgLabels_row_prev[c];
                }
                else {
                    // q = 0
                    if (condition_r) {
                        if (condition_p) {
                            // x <- merge(p,r)
                            imgLabels_row[c] = set_union(P, imgLabels_row_prev[c - 1], imgLabels_row_prev[c + 1]);
                        }
                        else {
                            // p = q = 0
                            if (condition_s) {
                                // x <- merge(s,r)
                                imgLabels_row[c] = set_union(P, imgLabels_row[c - 1], imgLabels_row_prev[c + 1]);
                            }
                            else {
                                // p = q = s = 0
                                // x <- r
                                imgLabels_row[c] = imgLabels_row_prev[c + 1];
                            }
                        }
                    }
                    else {
                        // r = q = 0
                        if (condition_p) {
                            // x <- p
                            imgLabels_row[c] = imgLabels_row_prev[c - 1];
                        }
                        else {
                            // r = q = p = 0
                            if (condition_s) {
                                imgLabels_row[c] = imgLabels_row[c - 1];
                            }
                            else {
                                //new label
                                imgLabels_row[c] = lunique;
                                P[lunique] = lunique;
                                lunique = lunique + 1;
                            }
                        }
                    }
                }
            }
            else {
                //Nothing to do, x is a background pixel
            }
        }
    }

    return lunique;
}

unsigned SAUF::SecondScan(const unsigned& lunique)
{
    int nLabels = flattenL(P, lunique);

    //second scan
    for (int r = 0; r < aImgLabels.rows; ++r) {
        unsigned * img_row_start = aImgLabels.ptr<unsigned>(r);
        unsigned * const img_row_end = img_row_start + aImgLabels.cols;
        for (; img_row_start != img_row_end; ++img_row_start) {
            *img_row_start = P[*img_row_start];
        }
    }

    return nLabels;
}

unsigned SAUF::PerformLabeling()
{
    const int h = aImg.rows;
    const int w = aImg.cols;

    aImgLabels = Mat1i(aImg.size(), 0);

    //const size_t Plength = (size_t)((aImg.rows + 1) / 2) * (size_t)((aImg.cols + 1) / 2) + 1;
    //P = (unsigned *)fastMalloc(sizeof(unsigned)* Plength); //array P for equivalences resolution
    P[0] = 0;	//first label is for background pixels
    unsigned lunique = 1;

    // Rosenfeld Mask
    // +-+-+-+
    // |p|q|r|
    // +-+-+-+
    // |s|x|
    // +-+-+

    // first scan
    for (int r = 0; r < h; ++r) {
        // Get row pointers
        uchar const * const img_row = aImg.ptr<uchar>(r);
        uchar const * const img_row_prev = (uchar *)(((char *)img_row) - aImg.step.p[0]);
        unsigned * const  imgLabels_row = aImgLabels.ptr<unsigned>(r);
        unsigned * const  imgLabels_row_prev = (unsigned *)(((char *)imgLabels_row) - aImgLabels.step.p[0]);

        for (int c = 0; c < w; ++c) {
        #define condition_p c>0 && r>0 && img_row_prev[c - 1]>0
        #define condition_q r>0 && img_row_prev[c]>0
        #define condition_r c < w - 1 && r > 0 && img_row_prev[c + 1] > 0
        #define condition_s c > 0 && img_row[c - 1] > 0
        #define condition_x img_row[c] > 0

            if (condition_x) {
                if (condition_q) {
                    //x <- q
                    imgLabels_row[c] = imgLabels_row_prev[c];
                }
                else {
                    // q = 0
                    if (condition_r) {
                        if (condition_p) {
                            // x <- merge(p,r)
                            imgLabels_row[c] = set_union(P, imgLabels_row_prev[c - 1], imgLabels_row_prev[c + 1]);
                        }
                        else {
                            // p = q = 0
                            if (condition_s) {
                                // x <- merge(s,r)
                                imgLabels_row[c] = set_union(P, imgLabels_row[c - 1], imgLabels_row_prev[c + 1]);
                            }
                            else {
                                // p = q = s = 0
                                // x <- r
                                imgLabels_row[c] = imgLabels_row_prev[c + 1];
                            }
                        }
                    }
                    else {
                        // r = q = 0
                        if (condition_p) {
                            // x <- p
                            imgLabels_row[c] = imgLabels_row_prev[c - 1];
                        }
                        else {
                            // r = q = p = 0
                            if (condition_s) {
                                imgLabels_row[c] = imgLabels_row[c - 1];
                            }
                            else {
                                //new label
                                imgLabels_row[c] = lunique;
                                P[lunique] = lunique;
                                lunique = lunique + 1;
                            }
                        }
                    }
                }
            }
            else {
                //Nothing to do, x is a background pixel
            }
        }
    }

    //second scan
    unsigned nLabel = flattenL(P, lunique);

    for (int r = 0; r < aImgLabels.rows; ++r) {
        unsigned * img_row_start = aImgLabels.ptr<unsigned>(r);
        unsigned * const img_row_end = img_row_start + aImgLabels.cols;
        for (; img_row_start != img_row_end; ++img_row_start) {
            *img_row_start = P[*img_row_start];
        }
    }

    //fastFree(P);
    return nLabel;

#undef condition_p
#undef condition_q
#undef condition_r
#undef condition_s
#undef condition_x
}

unsigned SAUF::PerformLabelingMem(vector<unsigned long>& accesses)
{
    const int h = aImg.rows;
    const int w = aImg.cols;

    const size_t Plength = upperBound8Conn;

    //Data structure for memory test
    memMat<uchar> img(aImg);
    memMat<int> imgLabels(aImg.size(), 0);
    memVector<unsigned> P(Plength);						 // Vector P for equivalences resolution

    P[0] = 0;	//first label is for background pixels
    unsigned lunique = 1;

    //first scan

    //Rosenfeld Mask
    //+-+-+-+
    //|p|q|r|
    //+-+-+-+
    //|s|x|
    //+-+-+

    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
        #define condition_p c>0 && r>0 && img(r-1 , c-1)>0
        #define condition_q r>0 && img(r-1, c)>0
        #define condition_r c < w - 1 && r > 0 && img(r-1,c+1)>0
        #define condition_s c > 0 && img(r,c-1)>0
        #define condition_x img(r,c)>0

            if (condition_x) {
                if (condition_q) {
                    //x <- q
                    imgLabels(r, c) = imgLabels(r - 1, c);
                }
                else {
                    //q = 0
                    if (condition_r) {
                        if (condition_p) {
                            //x <- merge(p,r)
                            imgLabels(r, c) = set_union(P, (unsigned)imgLabels(r - 1, c - 1), (unsigned)imgLabels(r - 1, c + 1));
                        }
                        else {
                            //p = q = 0
                            if (condition_s) {
                                //x <- merge(s,r)
                                imgLabels(r, c) = set_union(P, (unsigned)imgLabels(r, c - 1), (unsigned)imgLabels(r - 1, c + 1));
                            }
                            else {
                                //p = q = s = 0
                                //x <- r
                                imgLabels(r, c) = imgLabels(r - 1, c + 1);
                            }
                        }
                    }
                    else {
                        //r = q = 0
                        if (condition_p) {
                            //x <- p
                            imgLabels(r, c) = imgLabels(r - 1, c - 1);
                        }
                        else {
                            //r = q = p = 0
                            if (condition_s) {
                                imgLabels(r, c) = imgLabels(r, c - 1);
                            }
                            else {
                                //new label
                                imgLabels(r, c) = lunique;
                                P[lunique] = lunique;
                                lunique = lunique + 1;
                            }
                        }
                    }
                }
            }
            else {
                //Nothing to do, x is a background pixel
            }
        }
    }

    //second scan
    unsigned nLabel = flattenL(P, lunique);

    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            imgLabels(r, c) = P[imgLabels(r, c)];
        }
    }

    //Store total accesses in the output vector 'accesses'
    accesses = vector<unsigned long int>((int)MD_SIZE, 0);

    accesses[MD_BINARY_MAT] = (unsigned long int)img.getTotalAcesses();
    accesses[MD_LABELED_MAT] = (unsigned long int)imgLabels.getTotalAcesses();
    accesses[MD_EQUIVALENCE_VEC] = (unsigned long int)P.getTotalAcesses();

    //a = imgLabels.getImage();

    return nLabel;

#undef condition_p
#undef condition_q
#undef condition_r
#undef condition_s
#undef condition_x
}

void SAUF2::AllocateMemory()
{
    const size_t Plength = upperBound8Conn;
    P = (unsigned *)fastMalloc(sizeof(unsigned) * Plength); //array P for equivalences resolution
    return;
}

void SAUF2::DeallocateMemory()
{
    fastFree(P);
    return;
}

unsigned SAUF2::PerformLabeling()
{
    const int h = aImg.rows;
    const int w = aImg.cols;

    aImgLabels = Mat1i(aImg.size(), 0);

    //const size_t Plength = (size_t)((aImg.rows + 1) / 2) * (size_t)((aImg.cols + 1) / 2) + 1;
    //P = (unsigned *)fastMalloc(sizeof(unsigned)* Plength); //array P for equivalences resolution
    P[0] = 0;	//first label is for background pixels
    unsigned lunique = 1;

    // Rosenfeld Mask
    // +-+-+-+
    // |p|q|r|
    // +-+-+-+
    // |s|x|
    // +-+-+

    // first scan
    for (int r = 0; r < h; ++r) {
        // Get row pointers
        uchar const * const img_row = aImg.ptr<uchar>(r);
        uchar const * const img_row_prev = (uchar *)(((char *)img_row) - aImg.step.p[0]);
        unsigned * const  imgLabels_row = aImgLabels.ptr<unsigned>(r);
        unsigned * const  imgLabels_row_prev = (unsigned *)(((char *)imgLabels_row) - aImgLabels.step.p[0]);

        for (int c = 0; c < w; ++c) {
        #define condition_p c>0 && r>0 && img_row_prev[c - 1]>0
        #define condition_q r>0 && img_row_prev[c]>0
        #define condition_r c < w - 1 && r > 0 && img_row_prev[c + 1] > 0
        #define condition_s c > 0 && img_row[c - 1] > 0
        #define condition_x img_row[c] > 0

            if (condition_x) {
                if (condition_q) {
                    //x <- q
                    imgLabels_row[c] = imgLabels_row_prev[c];
                }
                else {
                    // q = 0
                    if (condition_r) {
                        if (condition_p) {
                            // x <- merge(p,r)
                            imgLabels_row[c] = MergeRemSp(P, imgLabels_row_prev[c - 1], imgLabels_row_prev[c + 1]);
                        }
                        else {
                            // p = q = 0
                            if (condition_s) {
                                // x <- merge(s,r)
                                imgLabels_row[c] = MergeRemSp(P, imgLabels_row[c - 1], imgLabels_row_prev[c + 1]);
                            }
                            else {
                                // p = q = s = 0
                                // x <- r
                                imgLabels_row[c] = imgLabels_row_prev[c + 1];
                            }
                        }
                    }
                    else {
                        // r = q = 0
                        if (condition_p) {
                            // x <- p
                            imgLabels_row[c] = imgLabels_row_prev[c - 1];
                        }
                        else {
                            // r = q = p = 0
                            if (condition_s) {
                                imgLabels_row[c] = imgLabels_row[c - 1];
                            }
                            else {
                                //new label
                                imgLabels_row[c] = lunique;
                                P[lunique] = lunique;
                                lunique = lunique + 1;
                            }
                        }
                    }
                }
            }
            else {
                //Nothing to do, x is a background pixel
            }
        }
    }

    //second scan
    unsigned nLabel = flattenL(P, lunique);

    for (int r = 0; r < aImgLabels.rows; ++r) {
        unsigned * img_row_start = aImgLabels.ptr<unsigned>(r);
        unsigned * const img_row_end = img_row_start + aImgLabels.cols;
        for (; img_row_start != img_row_end; ++img_row_start) {
            *img_row_start = P[*img_row_start];
        }
    }

    //fastFree(P);
    return nLabel;

#undef condition_p
#undef condition_q
#undef condition_r
#undef condition_s
#undef condition_x
}