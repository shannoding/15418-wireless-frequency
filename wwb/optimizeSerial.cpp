#include "main.h"

void get_full_band(const int N, 
                const int band_low[], 
                const int band_high[], int &global_low, int &global_high) {
    int min = FREQ_MAX;
    int max = FREQ_MIN;
    for (int i = 0; i < N; i++) {
        int low = band_low[i];
        min = (low < min) ? low : min;

        int high = band_high[i];
        max = (high > max) ? high : max;
    }
    global_low = min;
    global_high = max;
}

int inline get_rand_freq(int band_low, int band_high) {
    return (rand() % ((band_high - band_low) / FREQ_SPACING + 1)) * FREQ_SPACING + band_low;
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
    return (f - global_low) / FREQ_SPACING;
}

static inline int index_has_im(int (&spectrum)[], int index, IMProduct im) {
    return spectrum[index * NUM_OF_IM_PRODUCTS + im];
}

static inline void set_index_with_im(int (&spectrum)[], int index, IMProduct im, int val) {
    spectrum[index * NUM_OF_IM_PRODUCTS + im] = val;
}

static inline bool check_self_chan_compatible_in_spectrum(int n, int fnew_index, const int (&compatibility)[], int S, int (&spectrum)[], bool set = false) {

    for (int im = 0; im < NUM_OF_IM_PRODUCTS; im++) {
        int compatibility_spacing_im = compatibility[n * NUM_OF_IM_PRODUCTS + im];
        int compatibility_spacing_im_index = compatibility_spacing_im / FREQ_SPACING - 1;

        int low_index = fnew_index - compatibility_spacing_im_index;
        int high_index = fnew_index + compatibility_spacing_im_index;
        low_index = (low_index < 0) ? 0 : low_index;
        high_index = (high_index >= S) ? S - 1 : high_index;


        for (int i = low_index; i <= high_index; i++) {
            int has_im_status = index_has_im(spectrum, i, (IMProduct) im);
            if (set) {
                set_index_with_im(spectrum, i, (IMProduct) im, -1);
            }
            else if (has_im_status == 1) {
                return false;
            }
        }
    }
    if (set) {
        set_index_with_im(spectrum, fnew_index, im_chan, 1);
    }
    return true;
}

inline bool within_freq_range(int global_low, int global_high, int f) {
    return f >= global_low && f <= global_high;
}

static inline bool check_existing_compatible_in_spectrum(
    int global_low, int global_high, 
    int fnew,
    const int N,
    const int n,
    int (&frequencies)[], // size = N_F
    int (&spectrum)[], bool set = false) {
        for (int i = 0; i < n; i++) {
            int f2 = frequencies[i];
            IMProduct ims[] = { im_2T3O, im_2T5O, im_2T7O, im_2T9O };
            for (auto im : ims) {
                int fc1 = check_im_product(fnew, f2, 0, im);
                int fc1_index = freq_to_index(global_low, global_high, fc1);


                int fc2 = check_im_product(f2, fnew, 0, im);
                int fc2_index = freq_to_index(global_low, global_high, fc2);
                if (within_freq_range(global_low, global_high, fc1)) {
                    if (set) {
                        set_index_with_im(spectrum, fc1_index, im, 1);
                    }
                    else if (index_has_im(spectrum, fc1_index, im) == -1) {
                        return false;
                    }
                }
                if (within_freq_range(global_low, global_high, fc2)) {
                    if (set) {
                        set_index_with_im(spectrum, fc2_index, im, 1);
                    }
                    else if (index_has_im(spectrum, fc2_index, im) == -1) {
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
                    if (set) {
                        set_index_with_im(spectrum, fc1_index, im, 1);
                    }
                    else if (index_has_im(spectrum, fc1_index, im) == -1) {
                        return false;
                    }
                }
                if (within_freq_range(global_low, global_high, fc2)) {
                    if (set) {
                        set_index_with_im(spectrum, fc2_index, im, 1);
                    }
                    else if (index_has_im(spectrum, fc2_index, im) == -1) {
                        return false;
                    }
                }
                if (within_freq_range(global_low, global_high, fc3)) {
                    if (set) {
                        set_index_with_im(spectrum, fc3_index, im, 1);
                    }
                    else if (index_has_im(spectrum, fc3_index, im) == -1) {
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

inline int get_max_attempts(const int band_low, const int band_high) {
    return (band_high - band_low) / FREQ_SPACING + 1;
}

bool update_spectrum(int global_low, int global_high, 
    int &next_frequency,
    const int N,
    const int n,
    int (&frequencies)[], // size = N_F
    const int (&compatibility)[], // size = N_F * N_IM
    const int (&band_low)[], // size = N_F
    const int (&band_high)[], // size = N_F
    int (&spectrum)[] // size (global_high - global_low) / SPACING * NUM_OF_IM_PRODUCTS
    ) 
{
    bool success = false;

    int S = ((global_high - global_low) / FREQ_SPACING + 1);

    int attempts = 0;

    int fnew = get_rand_freq(band_low[n], band_high[n]);
    int fnew_index;

    int max_attempts = get_max_attempts(band_low[n], band_high[n]);

    
    while (!success) {
        if (attempts > max_attempts) {
            next_frequency = 0;
            // std::cout << "too many attempts" << std::endl;
            return false;
        }
        attempts++;
        fnew = get_rand_freq(band_low[n], band_high[n]);
        //advance_freq(band_low[n], band_high[n], fnew, attempts); // get_rand_freq(band_low[n], band_high[n]);
        fnew_index = freq_to_index(global_low, global_high, fnew);
        // check for channel conflicts
        bool self_chan_compatible = check_self_chan_compatible_in_spectrum(n, fnew_index, compatibility, S, spectrum);
        if (!self_chan_compatible) {
            // std::cout << fnew << " failed self chan" << std::endl;

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
        success = true;

    }
    next_frequency = fnew;
    check_self_chan_compatible_in_spectrum(n, fnew_index, compatibility, S, spectrum, true);
    check_existing_compatible_in_spectrum(
        global_low, global_high, 
        fnew,
        N, n,
        frequencies,
        spectrum, true);
    // std::cout << "found frequency " << n << " in " << attempts << " attempts" << std::endl;

    return true;
}

void print_spectrum(int s, const int (&spectrum)[]) {
    for (int i = 0; i < s; i += NUM_OF_IM_PRODUCTS) {
        for (int j = 0; j < NUM_OF_IM_PRODUCTS; j++) {
            std::cout << spectrum[i + j] << " ";
        }
        std::cout << std::endl;
    }
}

int calculate_frequencies_serial(int N, 
                const int (&band_low)[], 
                const int (&band_high)[],
                int (&frequencies)[],
                const int (&compatibility)[]
                ) {
    int global_low;
    int global_high;

    get_full_band(N, band_low, band_high, global_low, global_high);
    
    int s = ((global_high - global_low) / FREQ_SPACING + 1) * NUM_OF_IM_PRODUCTS;
    int spectrum[s];

    for (int i = 0; i < s; i++) {
        spectrum[i] = 0;
    }

    for (int i = 0; i < N; i++) {
        int next_frequency;
        bool status = update_spectrum(global_low, global_high, 
        next_frequency,
        N,
        i,
        frequencies, // size = N_F
        compatibility, // size = N_F * N_IM
        band_low, 
        band_high,
        spectrum 
        );
        if (status) {
            frequencies[i] = next_frequency;
            // std::cout << "frequency " << i << " is " << frequencies[i] << std::endl;
            // print_spectrum(s, spectrum);
        }
        else {
            return i; // this is mic that didn't get fit
        }
        
    }
    
    return N;
}

static inline int index_to_freq(int global_low, int global_high, int index) {
    return global_low + index * FREQ_SPACING;
}


extern int calculate_frequencies_ispc_wrapper(int N, 
                int (&band_low)[], 
                int (&band_high)[],
                int (&frequencies)[],
                int (&compatibility)[]
                ) {
    int global_low;
    int global_high;

    get_full_band(N, band_low, band_high, global_low, global_high);
    
    int S = ((global_high - global_low) / FREQ_SPACING + 1); 
    int s = S * NUM_OF_IM_PRODUCTS;
    int spectrum[s];

    for (int i = 0; i < s; i++) {
        spectrum[i] = 0;
    }

    for (int i = 0; i < N; i++) {
        int max_attempts = get_max_attempts(band_low[i], band_high[i]);
        bool frequency_attempts[max_attempts];

        ispc::update_spectrum_ispc(
        global_low, global_high, 
        max_attempts,
        frequency_attempts,
        N, i,
        frequencies, // size = N_F
        compatibility, // size = N_F * N_IM
        band_low, 
        band_high,
        spectrum 
        );

        int fstart = get_rand_freq(band_low[i], band_high[i]);
        int fstart_index = freq_to_index(global_low, global_high, fstart);
        int band_low_index = freq_to_index(global_low, global_high, band_low[i]);
        int band_high_index = freq_to_index(global_low, global_high, band_high[i]);

        bool found = false;


        for (int a = 0; a < max_attempts; a++) {
            if (fstart_index > band_high_index) {
                fstart_index = band_low_index;
            }
            if (frequency_attempts[fstart_index]) {
                fstart = index_to_freq(global_low, global_high, fstart_index);
                found = true;
                break;
            }
            fstart_index++;
        }
        if (found) {
            frequencies[i] = fstart;
            // std::cout << "frequency " << i << " is " << frequencies[i] << std::endl;

            // apply the spectrum changes
            check_self_chan_compatible_in_spectrum(i, fstart_index, compatibility, S, spectrum, true);
            check_existing_compatible_in_spectrum(
                global_low, global_high, 
                fstart,
                N, i,
                frequencies,
                spectrum, true);

        }
        else {
            return i; // this is mic that didn't get fit
        }
        
    }
    
    return N;
}

extern int calculate_frequencies_ispc_tasks_wrapper(int N, 
                int (&band_low)[], 
                int (&band_high)[],
                int (&frequencies)[],
                int (&compatibility)[]
                ) {
    int global_low;
    int global_high;

    get_full_band(N, band_low, band_high, global_low, global_high);
    
    int S = ((global_high - global_low) / FREQ_SPACING + 1); 
    int s = S * NUM_OF_IM_PRODUCTS;
    int spectrum[s];

    for (int i = 0; i < s; i++) {
        spectrum[i] = 0;
    }

    for (int i = 0; i < N; i++) {
        int max_attempts = get_max_attempts(band_low[i], band_high[i]);
        bool frequency_attempts[max_attempts];

        ispc::update_spectrum_ispc_withtasks(
        global_low, global_high, 
        max_attempts,
        frequency_attempts,
        N, i,
        frequencies, // size = N_F
        compatibility, // size = N_F * N_IM
        band_low, 
        band_high,
        spectrum 
        );

        int fstart = get_rand_freq(band_low[i], band_high[i]);
        int fstart_index = freq_to_index(global_low, global_high, fstart);
        int band_low_index = freq_to_index(global_low, global_high, band_low[i]);
        int band_high_index = freq_to_index(global_low, global_high, band_high[i]);

        bool found = false;


        for (int a = 0; a < max_attempts; a++) {
            if (fstart_index > band_high_index) {
                fstart_index = band_low_index;
            }
            if (frequency_attempts[fstart_index]) {
                fstart = index_to_freq(global_low, global_high, fstart_index);
                found = true;
                break;
            }
            fstart_index++;
        }
        if (found) {
            frequencies[i] = fstart;
            // std::cout << "frequency " << i << " is " << frequencies[i] << std::endl;

            // apply the spectrum changes
            check_self_chan_compatible_in_spectrum(i, fstart_index, compatibility, S, spectrum, true);
            check_existing_compatible_in_spectrum(
                global_low, global_high, 
                fstart,
                N, i,
                frequencies,
                spectrum, true);

        }
        else {
            return i; // this is mic that didn't get fit
        }
        
    }
    
    return N;
}