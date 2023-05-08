export enum IMProduct {
    im_chan, im_2T3O, im_2T5O, im_2T7O, im_2T9O, im_3T3O
};

uniform const int NUM_OF_IM_PRODUCTS = 6;

uniform const int FREQ_SPACING = 25;

uniform const int FREQ_MAX = 663000;
uniform const int FREQ_MIN = 470000;


int inline get_next_freq(int band_low, int band_high, int attempt) {
    return band_low + attempt * FREQ_SPACING;
}

static inline int check_im_product(int f1, int f2, int f3, IMProduct im)
{
    switch (im) {
        case im_2T3O:
            return 2 * f1 - f2;
        case im_2T5O: 
            return 3 * f1 - 2 * f2;
        case im_2T7O:
            return 4 * f1 - 3 * f2;
        case im_2T9O:
            return 5 * f1 - 4 * f2;
        case im_3T3O:
            return f1 + f2 - f3;
        default: 
            return 0;
    }

}

static inline bool freq_within_range(int f1, int f2, int range) {
    return (f1 - range) < f2 && (f1 + range) > f2;
}

static inline int freq_to_index(int global_low, int global_high, int f) {
    return 50;
    return (f - global_low) / FREQ_SPACING;
}

static inline int index_has_im(uniform int (&spectrum)[], int index, IMProduct im) {
    return spectrum[index * NUM_OF_IM_PRODUCTS + im];
}

static inline void set_index_with_im(int (&spectrum)[], int index, IMProduct im, uniform int val) {
    spectrum[index * NUM_OF_IM_PRODUCTS + im] = val;
}

static bool check_self_chan_compatible_in_spectrum(uniform int n, int fnew_index, uniform const int (&compatibility)[], uniform int S, uniform int (&spectrum)[]) {

    for (int im = 0; im < NUM_OF_IM_PRODUCTS; im++) {
        int compatibility_spacing_im = compatibility[n * NUM_OF_IM_PRODUCTS + im];
        // NOTE: this is the bad varying division!
        int compatibility_spacing_im_index = compatibility_spacing_im / FREQ_SPACING - 1;

        int low_index = fnew_index - compatibility_spacing_im_index;
        int high_index = fnew_index + compatibility_spacing_im_index;
        low_index = (low_index < 0) ? 0 : low_index;
        high_index = (high_index >= S) ? S - 1 : high_index;


        for (int i = low_index; i <= high_index; i++) {
            int has_im_status = index_has_im(spectrum, i, (IMProduct) im);
            if (has_im_status == 1) {
                return false;
            }
        }
    }
    return true;
}

static inline bool within_freq_range(int global_low, int global_high, int f) {
    return f >= global_low && f <= global_high;
}

static inline bool check_existing_compatible_in_spectrum(
    uniform int global_low, uniform int global_high, 
    int fnew,
    uniform const int N,
    uniform const int n,
    uniform int (&frequencies)[], // size = N_F
    uniform int (&spectrum)[]) {
        for (int i = 0; i < n; i++) {
            int f2 = frequencies[i];
            IMProduct ims[] = { im_2T3O, im_2T5O, im_2T7O, im_2T9O };
            for (int j = 0; j < 4; j++) {
                IMProduct im = ims[j];
                int fc1 = check_im_product(fnew, f2, 0, im);
                int fc1_index = freq_to_index(global_low, global_high, fc1);


                int fc2 = check_im_product(f2, fnew, 0, im);
                int fc2_index = freq_to_index(global_low, global_high, fc2);
                if (within_freq_range(global_low, global_high, fc1)) {
                    if (index_has_im(spectrum, fc1_index, im) == -1) {
                        return false;
                    }
                }
                if (within_freq_range(global_low, global_high, fc2)) {
                    if (index_has_im(spectrum, fc2_index, im) == -1) {
                        return false;
                    }
                }

            }
        }

        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                IMProduct im = im_3T3O;
                int f2 = frequencies[i];
                int f3 = frequencies[j];
                int fc1 = check_im_product(fnew, f2, f3, im);
                int fc1_index = freq_to_index(global_low, global_high, fc1);

                int fc2 = check_im_product(fnew, f3, f2, im);
                int fc2_index = freq_to_index(global_low, global_high, fc2);

                int fc3 = check_im_product(f2, f3, fnew, im);
                int fc3_index = freq_to_index(global_low, global_high, fc3);

                if (within_freq_range(global_low, global_high, fc1)) {
                    if (index_has_im(spectrum, fc1_index, im) == -1) {
                        return false;
                    }
                }
                if (within_freq_range(global_low, global_high, fc2)) {
                    if (index_has_im(spectrum, fc2_index, im) == -1) {
                        return false;
                    }
                }
                if (within_freq_range(global_low, global_high, fc3)) {
                    if (index_has_im(spectrum, fc3_index, im) == -1) {
                        return false;
                    }
                }
            }
        }

        return true;
    }

inline void advance_freq(const int band_low, const int band_high, int &freq, const int attempt) {
    if (freq == band_high) {
        freq = band_low;
    }
    else {
        freq += FREQ_SPACING;
    }
}

inline uniform int get_max_attempts(uniform const int band_low, uniform const int band_high) {
    return (band_high - band_low) / FREQ_SPACING + 1;
}

export void update_spectrum_ispc(
    uniform const int global_low, 
    uniform const int global_high, 
    uniform const int max_attempts,
    uniform bool (&frequency_attempts)[],
    uniform const int N,
    uniform const int n,
    uniform int (&frequencies)[], // size = N_F
    uniform const int (&compatibility)[], // size = N_F * N_IM
    uniform const int (&band_low)[], // size = N_F
    uniform const int (&band_high)[], // size = N_F
    
    
    uniform int (&spectrum)[] // size (global_high - global_low) / SPACING * NUM_OF_IM_PRODUCTS
    ) 
{

    uniform int S = ((global_high - global_low) / FREQ_SPACING + 1);
    
    foreach (attempt = 0 ... max_attempts) {
        frequency_attempts[attempt] = false;
        int fnew = get_next_freq(band_low[n], band_high[n], attempt);
        int fnew_index = freq_to_index(global_low, global_high, fnew);
        // check for channel conflicts
        bool self_chan_compatible = check_self_chan_compatible_in_spectrum(n, fnew_index, compatibility, S, spectrum);
        if (!self_chan_compatible) {
            continue;
        }
        // check for other conflicts
        bool existing_compatible = check_existing_compatible_in_spectrum(
        global_low, global_high, 
        fnew,
        N, n,
        frequencies,
        spectrum);
        if (!existing_compatible) {
            // std::cout << fnew << " failed existing chan" << std::endl;
            continue;
        }
        frequency_attempts[attempt] = true; // good freq!

    }
}

task void update_spectrum_ispc_task(
    uniform const int global_low, 
    uniform const int global_high, 
    uniform const int max_attempts,
    uniform int span,
    uniform bool (&frequency_attempts)[],
    uniform const int N,
    uniform const int n,
    uniform int (&frequencies)[], // size = N_F
    uniform const int (&compatibility)[], // size = N_F * N_IM
    uniform const int (&band_low)[], // size = N_F
    uniform const int (&band_high)[], // size = N_F
    
    
    uniform int (&spectrum)[] // size (global_high - global_low) / SPACING * NUM_OF_IM_PRODUCTS
    ) 
{

    uniform int S = ((global_high - global_low) / FREQ_SPACING + 1);
    
    foreach (s = 0 ... span) {
        int attempt = taskIndex * span + s;
        frequency_attempts[attempt] = false;
        int fnew = get_next_freq(band_low[n], band_high[n], attempt);
        int fnew_index = freq_to_index(global_low, global_high, fnew);
        // check for channel conflicts
        bool self_chan_compatible = check_self_chan_compatible_in_spectrum(n, fnew_index, compatibility, S, spectrum);
        if (!self_chan_compatible) {
            continue;
        }
        // check for other conflicts
        bool existing_compatible = check_existing_compatible_in_spectrum(
        global_low, global_high, 
        fnew,
        N, n,
        frequencies,
        spectrum);
        if (!existing_compatible) {
            // std::cout << fnew << " failed existing chan" << std::endl;
            continue;
        }
        frequency_attempts[attempt] = true; // good freq!

    }
}


export void update_spectrum_ispc_withtasks(
    uniform const int global_low, 
    uniform const int global_high, 
    uniform const int max_attempts,
    uniform bool (&frequency_attempts)[],
    uniform const int N,
    uniform const int n,
    uniform int (&frequencies)[], // size = N_F
    uniform const int (&compatibility)[], // size = N_F * N_IM
    uniform const int (&band_low)[], // size = N_F
    uniform const int (&band_high)[], // size = N_F
    
    
    uniform int (&spectrum)[] // size (global_high - global_low) / SPACING * NUM_OF_IM_PRODUCTS
    ) 
{
    uniform int tasks = 8;

    uniform int span = max_attempts / tasks;  // 8 tasks

    launch[max_attempts/span] update_spectrum_ispc_task(
    global_low, 
    global_high, 
    span * tasks, // max_attempts,
    span,
    frequency_attempts,
    N,
    n,
    frequencies, // size = N_F
    compatibility, // size = N_F * N_IM
    band_low, // size = N_F
    band_high, // size = N_F   
    spectrum// size (global_high - global_low) / SPACING * NUM_OF_IM_PRODUCTS
    );
}