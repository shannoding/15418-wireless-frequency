#include <stdio.h>
#include <algorithm>
#include <bitset>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h> 

#include "CycleTimer.h"
#include "analyze_ispc.h"
#include "optimize_ispc.h"

using namespace ispc;

// C++ code
extern "C" {}

// enum IMProduct {
//     im_chan, im_2T3O, im_2T5O, im_2T7O, im_2T9O, im_3T3O
// };

const int NUM_OF_IM_PRODUCTS = 6;

const int FREQ_SPACING = 25;

const int FREQ_MAX = 663000;
const int FREQ_MIN = 470000;

// all frequency units are in 0.xxx kHz
struct Mic {
    int frequency;
    int channel_spacing;
    int intermod_spacing;
    int band_low;
    int band_high;
};

typedef struct Mic Mic;

extern bool quick_check_compatibility_arith_serial(const int N,
                            const int num_im,
                            const int (&frequencies)[], // size = N_F
                            const int (&compatibility)[], // size = N_F * N_IM
                            const IMProduct (&ims)[], // size = N_IM
                            int result[]);

extern int calculate_frequencies_serial(int N, 
                const int (&band_low)[], 
                const int (&band_high)[],
                int (&frequencies)[],
                const int (&compatibility)[]
                );

extern int calculate_frequencies_ispc_wrapper(int N, 
                int (&band_low)[], 
                int (&band_high)[],
                int (&frequencies)[],
                int (&compatibility)[]
                );

extern int calculate_frequencies_ispc_tasks_wrapper(int N, 
                int (&band_low)[], 
                int (&band_high)[],
                int (&frequencies)[],
                int (&compatibility)[]
                );

