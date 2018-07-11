/*
 * dsp.hpp
 *
 *  Created on: 02 окт. 2015 г.
 *      Author: sadko
 */

#ifndef CORE_DSP_H_
#define CORE_DSP_H_

#include <core/types.h>
#include <stddef.h>
#include <math.h>
#include <string.h>

#define DSP_OPTION_CPU_UNKNOWN          0UL
#define DSP_OPTION_CPU_INTEL            1UL
#define DSP_OPTION_CPU_AMD              2UL
#define DSP_OPTION_CPU_MASK             0x3UL
#define DSP_OPTION_FPU                  (1UL << 2)
#define DSP_OPTION_CMOV                 (1UL << 3)
#define DSP_OPTION_MMX                  (1UL << 4)
#define DSP_OPTION_SSE                  (1UL << 5)
#define DSP_OPTION_SSE2                 (1UL << 6)
#define DSP_OPTION_SSE3                 (1UL << 7)
#define DSP_OPTION_SSSE3                (1UL << 8)
#define DSP_OPTION_SSE4_1               (1UL << 9)
#define DSP_OPTION_SSE4_2               (1UL << 10)
#define DSP_OPTION_SSE4A                (1UL << 11)
#define DSP_OPTION_FMA3                 (1UL << 12)
#define DSP_OPTION_FMA4                 (1UL << 13)
#define DSP_OPTION_AVX                  (1UL << 14)
#define DSP_OPTION_AVX2                 (1UL << 15)

namespace lsp
{
    //-----------------------------------------------------------------------
    // DSP context parameters
    typedef struct dsp_context_t
    {
        uint32_t        top;
        uint32_t        data[15];
    } dsp_context_t;

    //-----------------------------------------------------------------------
    // Digital filtering
    #pragma pack(push, 1)
    /*
         Normalized biquad filter:
                   a0 + a1*z^-1 + a2*z^-2
           h[z] = ------------------------
                   1 - b1*z^-1 - b2*z^-2
     */

    // These constants should be redefined if structure of biquad_t changes
    #define BIQUAD_X8_A0_OFF    0x40
    #define BIQUAD_X8_A0_SOFF   "0x40"
    #define BIQUAD_X8_I0_OFF    0x50
    #define BIQUAD_X8_I0_SOFF   "0x50"
    #define BIQUAD_X8_A1_OFF    0x60
    #define BIQUAD_X8_A1_SOFF   "0x60"
    #define BIQUAD_X8_I1_OFF    0x70
    #define BIQUAD_X8_I1_SOFF   "0x70"
    #define BIQUAD_X8_A2_OFF    0x80
    #define BIQUAD_X8_A2_SOFF   "0x80"
    #define BIQUAD_X8_I2_OFF    0x90
    #define BIQUAD_X8_I2_SOFF   "0x90"
    #define BIQUAD_X8_B1_OFF    0xa0
    #define BIQUAD_X8_B1_SOFF   "0xa0"
    #define BIQUAD_X8_J1_OFF    0xb0
    #define BIQUAD_X8_J1_SOFF   "0xb0"
    #define BIQUAD_X8_B2_OFF    0xc0
    #define BIQUAD_X8_B2_SOFF   "0xc0"
    #define BIQUAD_X8_J2_OFF    0xd0
    #define BIQUAD_X8_J2_SOFF   "0xd0"

    #define BIQUAD_X4_A0_OFF    0x40
    #define BIQUAD_X4_A0_SOFF   "0x40"
    #define BIQUAD_X4_A1_OFF    0x50
    #define BIQUAD_X4_A1_SOFF   "0x50"
    #define BIQUAD_X4_A2_OFF    0x60
    #define BIQUAD_X4_A2_SOFF   "0x60"
    #define BIQUAD_X4_B1_OFF    0x70
    #define BIQUAD_X4_B1_SOFF   "0x70"
    #define BIQUAD_X4_B2_OFF    0x80
    #define BIQUAD_X4_B2_SOFF   "0x80"

    #define BIQUAD_X2_A_OFF     BIQUAD_X4_A0_OFF
    #define BIQUAD_X2_A_SOFF    BIQUAD_X4_A0_SOFF
    #define BIQUAD_X2_I_OFF     BIQUAD_X4_A1_OFF
    #define BIQUAD_X2_I_SOFF    BIQUAD_X4_A1_SOFF
    #define BIQUAD_X2_B_OFF     BIQUAD_X4_A2_OFF
    #define BIQUAD_X2_B_SOFF    BIQUAD_X4_A2_SOFF
    #define BIQUAD_X2_J_OFF     BIQUAD_X4_B1_OFF
    #define BIQUAD_X2_J_SOFF    BIQUAD_X4_B1_SOFF

    #define BIQUAD_X1_A_OFF     BIQUAD_X4_A0_OFF
    #define BIQUAD_X1_A_SOFF    BIQUAD_X4_A0_SOFF
    #define BIQUAD_X1_B_OFF     BIQUAD_X4_A1_OFF
    #define BIQUAD_X1_B_SOFF    BIQUAD_X4_A1_SOFF

    #define BIQUAD_D0_OFF       0x00
    #define BIQUAD_D0_SOFF      "0x00"
    #define BIQUAD_D1_OFF       0x10
    #define BIQUAD_D1_SOFF      "0x10"
    #define BIQUAD_D2_OFF       0x20
    #define BIQUAD_D2_SOFF      "0x20"
    #define BIQUAD_D3_OFF       0x30
    #define BIQUAD_D3_SOFF      "0x30"

    #define BIQUAD_D_ITEMS      16
    #define BIQUAD_ALIGN        0x40

    typedef struct biquad_x8_t
    {
        float   a0[8];
        float   a1[8];
        float   a2[8];
        float   b1[8];
        float   b2[8];
    } biquad_x8_t;

    typedef struct biquad_x4_t
    {
        float   a0[4];
        float   a1[4];
        float   a2[4];
        float   b1[4];
        float   b2[4];
    } biquad_x4_t;

    typedef struct biquad_x2_t
    {
        float   a[8];  //  a0 a0 a1 a2 i0 i1 i2 i3
        float   b[8];  //  b1 b2 0  0  j0 j1 j2 j3
    } biquad_x2_t;

    typedef struct biquad_x1_t
    {
        float   a[4];  //  a0 a0 a1 a2
        float   b[4];  //  b1 b2 0  0
    } biquad_x1_t;

    // This is main filter structure
    // It should be aligned at least to 16-byte boundary
    // For best purposes it should be aligned to 64-byte boundary
    typedef struct biquad_t
    {
        float   d[BIQUAD_D_ITEMS];
        union
        {
            biquad_x1_t x1;
            biquad_x2_t x2;
            biquad_x4_t x4;
            biquad_x8_t x8;
        };
        float   __pad[8];
    } __lsp_aligned(BIQUAD_ALIGN) biquad_t;

    #pragma pack(pop)

    //-----------------------------------------------------------------------
    // Resampling
    /** Resampling/oversampling funtion type.
     * Remember that destination buffer must be times greater and have additional gap (>=64 samples) at
     * the tail to contain complete convolution after resampling
     *
     * @param dst destination buffer
     * @param src source buffer
     * @param count number of samples to process
     */
    typedef void (* resampling_function_t)(float *dst, const float *src, size_t count);

    // Namespace containing function
    namespace dsp
    {
        // Start and finish types
        typedef void (* start_t)(dsp_context_t *ctx);
        typedef void (* finish_t)(dsp_context_t *ctx);

        /** Initialize DSP
         *
         */
        void init();

        /** Start DSP processing, save machine context
         *
         * @param ctx structure to save context
         */
        extern void (* start)(dsp_context_t *ctx);

        /** Finish DSP processing, restore machine context
         *
         * @param ctx structure to restore context
         */
        extern void (* finish)(dsp_context_t *ctx);

        /** Copy data: dst[i] = src[i]
         *
         * @param dst destination pointer
         * @param src source pointer
         * @param count number of elements
         */
        extern void (*copy)(float *dst, const float *src, size_t count);

        /** Copy data: dst[i] = saturate(src[i])
         *
         * @param dst destination pointer
         * @param src source pointer
         * @param count number of elements
         */
        extern void (*copy_saturated)(float *dst, const float *src, size_t count);

        /** Saturate data: dst[i] = saturate(src[i])
         *
         * @param dst destination pointer
         * @param src source pointer
         * @param count number of elements
         */
        extern void (* saturate)(float *dst, size_t count);

        /** Move data: dst[i] = src[i]
         *
         * @param dst destination pointer
         * @param src source pointer
         * @param count number of elements
         */
        extern void (*move)(float *dst, const float *src, size_t count);

        extern void (* fill)(float *dst, float value, size_t count);
        extern void (* fill_zero)(float *dst, size_t count);
        extern void (* fill_one)(float *dst, size_t count);
        extern void (* fill_minus_one)(float *dst, size_t count);

        /** Calculate absolute values: dst[i] = abs(src[i])
         *
         * @param dst destination vector
         * @param src source vector
         * @param count number of elements
         */
        extern void (* abs)(float *dst, const float *src, size_t count);

        /** Calculate absolute normalized values: dst[i] = abs(src[i]) / max { abs(src) }
         *
         * @param dst destination vector
         * @param src source vector
         * @param count number of elements
         */
        extern void (* abs_normalized)(float *dst, const float *src, size_t count);

        /** Calculate normalized values: dst[i] = src[i] / (max { abs(src) })
         *
         * @param dst destination vector
         * @param src source vector
         * @param count number of elements
         */
        extern void (* normalize)(float *dst, const float *src, size_t count);

        /** Calculate min { src }
         *
         * @param src source vector
         * @param count number of elements
         * @return minimum value
         */
        extern float (* min)(const float *src, size_t count);

        /** Calculate max { src }
         *
         * @param src source vector
         * @param count number of elements
         * @return maximum value
         */
        extern float (* max)(const float *src, size_t count);

        /** Get absolute maximum: result = max { abs(src[i]) }
         *
         * @param src source array
         * @param count number of elements
         * @return result
         */
        extern float (* abs_max)(const float *src, size_t count);

        /** Get absolute minimum: result = min { abs(src[i]) }
         *
         * @param src source array
         * @param count number of elements
         * @return result
         */
        extern float (* abs_min)(const float *src, size_t count);

        /** Calculate min { src }, max { src }
         *
         * @param src source vector
         * @param count number of elements
         * @return maximum value
         */
        extern void (* minmax)(const float *src, size_t count, float *min, float *max);

        /** Calculate @ min { src }
         *
         * @param src source vector
         * @param count number of elements
         * @return minimum value index
         */
        extern size_t (* min_index)(const float *src, size_t count);

        /** Calculate @ max { src }
         *
         * @param src source vector
         * @param count number of elements
         * @return maximum value
         */
        extern size_t (* max_index)(const float *src, size_t count);

        /** Calculate @ max { abs(src) }
         *
         * @param src source
         * @param count number of samples
         * @return index of maximum element
         */
        extern size_t  (* abs_max_index)(const float *src, size_t count);

        /** Calculate @ min { abs(src) }
         *
         * @param src source
         * @param count number of samples
         * @return index of maximum element
         */
        extern size_t  (* abs_min_index)(const float *src, size_t count);

        /** Multiply: dst[i] = src[i] * k
         *
         * @param dst destination
         * @param src source
         * @param k multiplier
         * @param count number of elements
         */
        extern void (* scale)(float *dst, const float *src, float k, size_t count);

        /** Multiply: dst[i] = src1[i] * src2[i]
         *
         * @param dst destination
         * @param src1 first source
         * @param src2 second source
         * @param count number of elements
         */
        extern void (* multiply)(float *dst, const float *src1, const float *src2, size_t count);

        /** Calculate horizontal sum: result = sum (i) from 0 to count-1 src[i]
         *
         * @param src vector to summarize
         * @param count number of elements
         * @return status of operation
         */
        extern float (* h_sum)(const float *src, size_t count);

        /** Calculate horizontal sum: result = sum (i) from 0 to count-1 sqr(src[i])
         *
         * @param src vector to summarize
         * @param count number of elements
         * @return status of operation
         */
        extern float (* h_sqr_sum)(const float *src, size_t count);

        /** Calculate horizontal sum of absolute values: result = sum (i) from 0 to count-1 abs(src[i])
         *
         * @param src vector to summarize
         * @param count number of elements
         * @return status of operation
         */
        extern float (* h_abs_sum)(const float *src, size_t count);

        /** Calculate sum {from 0 to count-1} (a[i] * b[i])
         *
         * @param a first vector
         * @param b second vector
         * @param count number of elements
         * @return scalar multiplication
         */
        extern float (* scalar_mul)(const float *a, const float *b, size_t count);

        /** Calculate dst[i] = dst[i] * k + src[i] * p
         *
         * @param dst accumulator list
         * @param src change list
         * @param k keep value
         * @param p push value
         * @param count number of elements
         */
        extern void (* accumulate)(float *dst, const float *src, float k, float p, size_t count);

        /** Calculate dst[i] = dst[i] + src[i] * k
         *
         * @param dst destination array
         * @param src source array
         * @param k multiplier
         * @param count number of elements
         */
        extern void (* add_multiplied)(float *dst, const float *src, float k, size_t count);

        /** Calculate dst[i] = dst[i] - src[i] * k
         *
         * @param dst destination array
         * @param src source array
         * @param k multiplier
         * @param count number of elements
         */
        extern void (* sub_multiplied)(float *dst, const float *src, float k, size_t count);

        /** Calculate dst[i] = dst[i] + src[i]
         *
         * @param dst destination array
         * @param src source array
         * @param count number of elements
         */
        extern void (* add2)(float *dst, const float *src, size_t count);

        /** Calculate dst[i] = dst[i] - src[i]
         *
         * @param dst destination array
         * @param src source array
         * @param count number of elements
         */
        extern void (* sub2)(float *dst, const float *src, size_t count);

        /** Calculate dst[i] = src1[i] + src2[i]
         *
         * @param dst destination array
         * @param src source array
         * @param count number of elements
         */
        extern void (* add3)(float *dst, const float *src1, const float *src2, size_t count);

        /** Calculate dst[i] = src1[i] - src2[i]
         *
         * @param dst destination array
         * @param src source array
         * @param count number of elements
         */
        extern void (* sub3)(float *dst, const float *src1, const float *src2, size_t count);

        /** Calculate dst[i] = dst[i] * (1 - k) + src[i] * k = dst[i] + (src[i] - dst[i]) * k
         *
         * @param dst destination
         * @param src function value to integrate
         * @param k time factor
         * @param count
         */
        extern void (* integrate)(float *dst, const float *src, float k, size_t count);

        /** Calculate dst[i] = src1[i] * k1 + src2[i] * k2
         *
         * @param dst destination buffer
         * @param src1 source buffer 1
         * @param src2 source buffer 2
         * @param k1 multiplier 1
         * @param k2 multiplier 2
         */
        extern void (* mix)(float *dst, const float *src1, const float *src2, float k1, float k2, size_t count);

        /** Calculate dst[i] = dst[i] + src1[i] * k1 + src2[i] * k2
         *
         * @param dst destination buffer
         * @param src1 source buffer 1
         * @param src2 source buffer 2
         * @param k1 multiplier 1
         * @param k2 multiplier 2
         */
        extern void (* mix_add)(float *dst, const float *src1, const float *src2, float k1, float k2, size_t count);

        /** Calculate convolution for the current sample
         *
         * @param src source data
         * @param conv REVERSED convolution data
         * @param count length of convolution
         * @return sample value
         */
        extern float   (* convolve_single)(const float *src, const float *conv, size_t length);

        /** Apply direct convolution: dst[i] = sum from j=0 to j=length { src[i + j] * conv[i] }
         *
         * @param dst destination buffer
         * @param src source buffer
         * @param conv REVERSED convolution data
         * @param length convolution length
         * @param count number of samples to process
         */
        extern void (* convolve)(float *dst, const float *src, const float *conv, size_t length, size_t count);

        /** Reverse the order of samples: dst[i] <=> dst[count - i - 1]
         *
         * @param dst the buffer to reverse
         * @param count number of samples in buffer
         */
        extern void (* reverse)(float *dst, size_t count);

        /** Direct Fast Fourier Transform
         * @param dst_re real part of spectrum
         * @param dst_im imaginary part of spectrum
         * @param src_re real part of signal
         * @param src_im imaginary part of signal
         * @param rank the rank of FFT
         */
        extern void (* direct_fft)(float *dst_re, float *dst_im, const float *src_re, const float *src_im, size_t rank);

        /** Direct Fast Fourier Transform with packed complex data
         * @param dst complex spectrum [re, im, re, im ...]
         * @param src complex signal [re, im, re, im ...]
         * @param rank the rank of FFT
         */
        extern void (* packed_direct_fft)(float *dst, const float *src, size_t rank);

        /** Direct Fast Fourier Transform for convolution
         * @param dst complex spectrum [re, im, re, im ...]
         * @param src real signal, buffer is twice lower size than output (in elements)
         * @param rank the rank of FFT
         */
        extern void (* conv_direct_fft)(float *dst, const float *src, size_t rank);

        /** Reverse Fast Fourier transform
         * @param dst_re real part of signal
         * @param dst_im imaginary part of signal
         * @param src_re real part of spectrum
         * @param src_im imaginary part of spectrum
         * @param rank the rank of FFT
         */
        extern void (* reverse_fft)(float *dst_re, float *dst_im, const float *src_re, const float *src_im, size_t rank);

        /** Reverse Fast Fourier transform with packed complex data
         * @param dst complex signal [re, im, re, im ...]
         * @param src complex spectrum [re, im, re, im ...]
         * @param rank the rank of FFT
         */
        extern void (* packed_reverse_fft)(float *dst, const float *src, size_t rank);

//        /** Build 2x larger FFT from 2 FFTs located one after other
//         *
//         * @param dst_re real part of signal
//         * @param dst_im imaginary part of signal
//         * @param src_re real part of spectrum of 2x FFTs
//         * @param src_im imaginary part of spectrum of 2x FFTs
//         * @param rank current rank of FFT
//         */
//        extern void (* join_fft)(float *dst_re, float *dst_im, float *src_re, float *src_im, size_t rank);

        /** Normalize FFT coefficients
         *
         * @param dst_re target array for real part of signal
         * @param dst_im target array for imaginary part of signal
         * @param src_re real part of spectrum
         * @param src_im imaginary part of spectrum;
         * @param rank the rank of FFT
         */
        extern void (* normalize_fft)(float *dst_re, float *dst_im, const float *src_re, const float *src_im, size_t rank);

        /** Center FFT coefficients
         *
         * @param dst_re target array for real part of signal
         * @param dst_im target array for imaginary part of signal
         * @param src_re source array for real part of signal
         * @param src_im source array for imaginary part of signal
         * @param rank rank of FFT
         */
        extern void (* center_fft)(float *dst_re, float *dst_im, const float *src_re, const float *src_im, size_t rank);

        /** Leave only harmonics with positive frequencies
         *
         * @param dst_re target array for real part of signal
         * @param dst_im target array for imaginary part of signal
         * @param src_re source array for real part of signal
         * @param src_im source array for imaginary part of signal
         * @param rank rank of FFT
         */
        extern void (* combine_fft)(float *dst_re, float *dst_im, const float *src_re, const float *src_im, size_t rank);

        /** Calculate complex multiplication
         *
         * @param dst_re destination real part
         * @param dst_im destination imaginary part
         * @param src1_re source 1 real part
         * @param src1_im source 1 imaginary part
         * @param src2_re source 2 real part
         * @param src2_im source 2 imaginary part
         * @param count number of multiplications
         */
        extern void (* complex_mul)(
                float *dst_re, float *dst_im,
                const float *src1_re, const float *src1_im,
                const float *src2_re, const float *src2_im,
                size_t count
            );

        /** Calculate packed complex multiplication
         *
         * @param dst destination to store complex numbers
         * @param src1 source 1
         * @param src2 source 2
         * @param count number of multiplications
         */
        extern void (* packed_complex_mul)(float *dst, const float *src1, const float *src2, size_t count);

        /** Convert real to packed complex
         *
         * @param dst destination packed complex data
         * @param src source real data
         * @param count number of items to convert
         */
        extern void (* packed_real_to_complex)(float *dst, const float *src, size_t count);

        /** Convert packed complex to real
         *
         * @param dst destination real data
         * @param src source packed complex data
         * @param count number of items to convert
         */
        extern void (* packed_complex_to_real)(float *dst, const float *src, size_t count);

        /** Convert packed complex to real and add to destination buffer
         *
         * @param dst destination real data
         * @param src source packed complex data
         * @param count number of items to convert
         */
        extern void (* packed_complex_add_to_real)(float *dst, const float *src, size_t count);

        /** Convert real+imaginary complex number to polar form
         *
         * @param dst_mod module of the complex number
         * @param dst_arg argument of the complex number
         * @param src_re real part of complex number
         * @param src_im imaginary part of complex number
         * @param count number of elements to process
         */
        extern void (* complex_cvt2modarg)(
                float *dst_mod, float *dst_arg,
                const float *src_re, const float *src_im,
                size_t count
            );

        /** Get module for complex numbers
         *
         * @param dst_mod array to sore module
         * @param src_re real part of complex number
         * @param src_im imaginary part of complex number
         * @param count number of elements to process
         */
        extern void (* complex_mod)(
                float *dst_mod,
                const float *src_re, const float *src_im,
                size_t count
            );

        /** Convert polar-form of complex number to real+imaginary
         *
         * @param dst_re real part of complex number
         * @param dst_im imaginary part of complex number
         * @param src_mod module of the complex number
         * @param src_arg argument of the complex number
         * @param count number of elements to process
         */
        extern void (* complex_cvt2reim)(
                float *dst_re, float *dst_im,
                const float *src_mod, const float *src_arg,
                size_t count
            );

        /** Parse input real data to fast convolution data
         *
         * @param dst destination buffer of 2^(rank+1) floats
         * @param src source real data of 2^(rank-1) floats
         * @param rank the convolution rank
         */
        extern void (* fastconv_parse)(float *dst, const float *src, size_t rank);

        /** Parse input real data to fast convolution data,
         *  convolve with another convolution data,
         *  restore data to real data and add to output buffer
         *
         * @param dst target real data of 2^rank floats to store convolved data
         * @param tmp temporary buffer of 2^(rank+1) floats to store intermediate data
         * @param c fast convolution data of 2^(rank+1) floats to apply to the buffer
         * @param src source real data of 2^(rank-1) floats
         * @param rank the convolution rank
         */
        extern void (* fastconv_parse_apply)(float *dst, float *tmp, const float *c, const float *src, size_t rank);

        /** Restore convolution to real data,
         * modifies the source fast convolution data
         *
         * @param dst destination real data of 2^rank floats
         * @param src source fast convolution data of 2^(rank+1) floats
         * @param rank the convolution rank
         */
        extern void (* fastconv_restore)(float *dst, float *src, size_t rank);

        /** Convolve two convolutiona and restore data to real data
         * and add to output buffer
         *
         * @param dst target real data of 2%rank floats to store convolved data
         * @param tmp temporary buffer of 2^(rank+1) floats to store intermediate data
         * @param c1 fast convolution data of 2^(rank+1) floats
         * @param c2 fast convolution data of 2^(rank+1) floats
         * @param rank the convolution rank
         */
        extern void (* fastconv_apply)(float *dst, float *tmp, const float *c1, const float *c2, size_t rank);

        /** Convert stereo signal to mid-side signal
         *
         * @param m mid signal
         * @param s side signal
         * @param l left signal
         * @param r right signal
         * @param count number of samples to process
         */
        extern void (* lr_to_ms)(float *m, float *s, const float *l, const float *r, size_t count);

        /** Convert stereo signal to middle signal
         *
         * @param m mid signal
         * @param l left channel
         * @param r right channel
         * @param count number of samples to process
         */
        extern void (* lr_to_mid)(float *m, const float *l, const float *r, size_t count);

        /** Convert stereo signal to side signal
         *
         * @param s side signal
         * @param l left channel
         * @param r right channel
         * @param count number of samples to process
         */
        extern void (* lr_to_side)(float *s, const float *l, const float *r, size_t count);

        /** Convert mid-side signal to left-right signal
         *
         * @param l left signal
         * @param r right signal
         * @param m mid signal
         * @param s side signal
         * @param count number of samples to process
         */
        extern void (* ms_to_lr)(float *l, float *r, const float *m, const float *s, size_t count);

        /** Convert mid-side signal to left signal
         *
         * @param l left signal
         * @param m mid signal
         * @param s side signal
         * @param count number of samples to process
         */
        extern void    (* ms_to_left)(float *l, const float *m, const float *s, size_t count);

        /** Convert mid-side signal to right signal
         *
         * @param r right signal
         * @param m mid signal
         * @param s side signal
         * @param count number of samples to process
         */
        extern void    (* ms_to_right)(float *r, const float *m, const float *s, size_t count);

        /** Process biquad filter
         *
         * @param buf filter delay buffer
         * @param ir impulse response part of filter
         * @param sample sample to process
         * @return processed sample
         */
        extern float (* biquad_process)(float *buf, const float *ir, float sample);

        /** Process bi-quadratic filter for multiple samples
         *
         * @param dst destination samples
         * @param src source samples
         * @param count number of samples to process
         * @param buf filter delay buffer (4 samples)
         * @param ir impulse response part of filter (5 samples)
         */
        extern void (* biquad_process_multi)(float *dst, const float *src, size_t count, float *buf, const float *ir);

        /** Process bi-quadratic filter for multiple samples
         *
         * @param dst destination samples
         * @param src source samples
         * @param count number of samples to process
         * @param f bi-quadratic filter structure
         */
        extern void (* biquad_process_x1)(float *dst, const float *src, size_t count, biquad_t *f);

        /** Process two bi-quadratic filters for multiple samples simultaneously
         *
         * @param dst destination samples
         * @param src source samples
         * @param count number of samples to process
         * @param f bi-quadratic filter structure
         */
        extern void (* biquad_process_x2)(float *dst, const float *src, size_t count, biquad_t *f);

        /** Avoid denormal values
         *
         * @param dst destination buffer
         * @param src source buffer
         * @param count number of samples
         */
        extern void (* avoid_denormals)(float *dst, const float *src, size_t count);

        /** Process four bi-quadratic filters for multiple samples simultaneously
         *
         * @param dst destination samples
         * @param src source samples
         * @param count number of samples to process
         * @param f bi-quadratic filter structure
         */
        extern void (* biquad_process_x4)(float *dst, const float *src, size_t count, biquad_t *f);

        /** Process eight bi-quadratic filters for multiple samples simultaneously
         *
         * @param dst destination samples
         * @param src source samples
         * @param count number of samples to process
         * @param f bi-quadratic filter structure
         */
        extern void (* biquad_process_x8)(float *dst, const float *src, size_t count, biquad_t *f);

        /** Do logarithmic vector apply:
         *  x[i] = x[i] + norm_x * logf(absf(v[i]*zero))
         *  y[i] = y[i] + norm_y * logf(absf(v[i]*zero))
         *
         * @param x destination vector for X coordinate
         * @param y destination vector for Y coordinate
         * @param v delta vector to apply
         * @param zero graphics zero point
         * @param norm_x X norming factor
         * @param norm_y Y norming factor
         */
        extern void (* axis_apply_log)(float *x, float *y, const float *v, float zero, float norm_x, float norm_y, size_t count);

        /** Convert RGBA32 -> BGRA32 color
         *
         * @param dst target buffer
         * @param src source buffer
         * @param count number of samples to process
         */
        extern void (* rgba32_to_bgra32)(void *dst, const void *src, size_t count);

        /** Perform lanczos resampling, destination buffer must be cleared and contain only
         * convolution tail from previous resampling
         *
         * @param dst destination buffer of count*2 samples + 64 samples for convolution tail
         * @param src source buffer of count samples
         * @param count number of samples
         */
        extern void (* lanczos_resample_2x2)(float *dst, const float *src, size_t count);

        /** Perform lanczos resampling, destination buffer must be cleared and contain only
         * convolution tail from previous resampling
         *
         * @param dst destination buffer of count*2 samples + 64 samples for convolution tail
         * @param src source buffer of count samples
         * @param count number of samples
         */
        extern void (* lanczos_resample_2x3)(float *dst, const float *src, size_t count);

        /** Perform lanczos resampling, destination buffer must be cleared and contain only
         * convolution tail from previous resampling
         *
         * @param dst destination buffer of count*3 samples + 64 samples for convolution tail
         * @param src source buffer of count samples
         * @param count number of samples
         */
        extern void (* lanczos_resample_3x2)(float *dst, const float *src, size_t count);

        /** Perform lanczos resampling, destination buffer must be cleared and contain only
         * convolution tail from previous resampling
         *
         * @param dst destination buffer of count*3 samples + 64 samples for convolution tail
         * @param src source buffer of count samples
         * @param count number of samples
         */
        extern void (* lanczos_resample_3x3)(float *dst, const float *src, size_t count);

        /** Perform lanczos resampling, destination buffer must be cleared and contain only
         * convolution tail from previous resampling
         *
         * @param dst destination buffer of count*4 samples + 64 samples for convolution tail
         * @param src source buffer of count samples
         * @param count number of samples
         */
        extern void (* lanczos_resample_4x2)(float *dst, const float *src, size_t count);

        /** Perform lanczos resampling, destination buffer must be cleared and contain only
         * convolution tail from previous resampling
         *
         * @param dst destination buffer of count*4 samples + 64 samples for convolution tail
         * @param src source buffer of count samples
         * @param count number of samples
         */
        extern void (* lanczos_resample_4x3)(float *dst, const float *src, size_t count);

        /** Perform lanczos resampling, destination buffer must be cleared and contain only
         * convolution tail from previous resampling
         *
         * @param dst destination buffer of count*6 samples + 64 samples for convolution tail
         * @param src source buffer of count samples
         * @param count number of samples
         */
        extern void (* lanczos_resample_6x2)(float *dst, const float *src, size_t count);

        /** Perform lanczos resampling, destination buffer must be cleared and contain only
         * convolution tail from previous resampling
         *
         * @param dst destination buffer of count*6 samples + 64 samples for convolution tail
         * @param src source buffer of count samples
         * @param count number of samples
         */
        extern void (* lanczos_resample_6x3)(float *dst, const float *src, size_t count);

        /** Perform lanczos resampling, destination buffer must be cleared and contain only
         * convolution tail from previous resampling
         *
         * @param dst destination buffer of count*8 samples + 64 samples for convolution tail
         * @param src source buffer of count samples
         * @param count number of samples
         */
        extern void (* lanczos_resample_8x2)(float *dst, const float *src, size_t count);

        /** Perform lanczos resampling, destination buffer must be cleared and contain only
         * convolution tail from previous resampling
         *
         * @param dst destination buffer of count*8 samples + 64 samples for convolution tail
         * @param src source buffer of count samples
         * @param count number of samples
         */
        extern void (* lanczos_resample_8x3)(float *dst, const float *src, size_t count);

        /** Copy each even sample to output buffer
         *
         * @param dst destination buffer
         * @param src source buffer
         * @param count number of samples to process
         */
        extern void (* downsample_2x)(float *dst, const float *src, size_t count);

        /** Copy each 3rd sample to output buffer
         *
         * @param dst destination buffer
         * @param src source buffer
         * @param count number of samples to process
         */
        extern void (* downsample_3x)(float *dst, const float *src, size_t count);

        /** Copy each 4th sample to output buffer
         *
         * @param dst destination buffer
         * @param src source buffer
         * @param count number of samples to process
         */
        extern void (* downsample_4x)(float *dst, const float *src, size_t count);

        /** Copy each 6th sample to output buffer
         *
         * @param dst destination buffer
         * @param src source buffer
         * @param count number of samples to process
         */
        extern void (* downsample_6x)(float *dst, const float *src, size_t count);

        /** Copy each 8th sample to output buffer
         *
         * @param dst destination buffer
         * @param src source buffer
         * @param count number of samples to process
         */
        extern void (* downsample_8x)(float *dst, const float *src, size_t count);
    }

} /* namespace forzee */

#endif /* CORE_DSP_HPP_ */
