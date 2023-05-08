#include <smmintrin.h> // For _mm_stream_load_si128
#include <emmintrin.h> // For _mm_mul_ps
#include <assert.h>
#include <stdint.h>

extern void saxpySerial(int N,
			float scale,
			float X[],
			float Y[],
			float result[]);


void saxpyStreaming(int N,
                    float scale,
                    float X[],
                    float Y[],
                    float result[])
{
    // Replace this code with ones that make use of the streaming instructions
    saxpySerial(N, scale, X, Y, result);
    // for (int i=0; i<N; i++) { 
    //     // __m128 res = _mm_mul_ps((__m128)scale, _mm_load_ss((__m128*)(X + i))) + _mm_load_ss((__m128*)(Y + i));
    //     // _mm_stream_si128((__m128*)(result + i), res);        
    //     // result[i] = scale * X[i] + Y[i];
    // }
}

