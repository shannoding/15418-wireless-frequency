#include "main.h"

static int check_im_product(int f1, int f2, int f3, IMProduct im)
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

static bool freq_within_range(int f1, int f2, int range) {
    return (f1 - range) < f2 && (f1 + range) > f2;
}

bool quick_check_compatibility_arith_serial(const int N,
                            const int num_im,
                            const int (&frequencies)[], // size = N_F
                            const int (&compatibility)[], // size = N_F * N_IM
                            const IMProduct (&ims)[], // size = N_IM
                            int result[])
{
    for (int i = 0; i < N; i++) {
        int f1 = frequencies[i];      
        const int *compatibility_for_f1 = &(compatibility[i]);

        // set result[i] = true if this frequency will experience a conflict
        // caused by someone else or an IM product
        int has_conflict = 0;

        for (int j = 0; j < N; j++) {

            int f2 = frequencies[j]; 

            if (i != j && freq_within_range(f1, f2, compatibility_for_f1[N * im_chan])) {
                has_conflict++;
                // std::cout << "mic " << i << " is conflict due to channel " << j << std::endl;

            }

            for (int k = 0; k < N; k++) {
                if (j == k) {
                    continue;
                }
                int f3 = frequencies[k];
                
                for (int im_i = 1; im_i < num_im; im_i++) {
                    IMProduct im = ims[im_i];
                    if (im == im_3T3O) {
                        for (int l = 0; l < N; l++) {
                            if (l == j || l == k) {
                                continue;
                            }
                            // all unique j, k, l
                            int f4 = frequencies[l];
                            int fc = check_im_product(f2, f3, f4, im);
                            if (freq_within_range(f1, fc, compatibility_for_f1[N * im])) {
                                has_conflict++;
                                // std::cout << "mic " << i << " is conflict with mics " << j << "," << k << "," << l << " due to im " << fc << " type " << im << " spacing " << compatibility_for_f1[im] << std::endl;


                            }
                        }
                    }
                    else {
                        int fc = check_im_product(f2, f3, 0, im);
                        // std::cout << "im prod " << fc << std::endl;

                        if (freq_within_range(f1, fc, compatibility_for_f1[N * im])) {
                            has_conflict++;
                            // std::cout << "mic " << i << " is conflict with mics " << j << " and " << k << " due to im " << fc << " type " << im << " spacing " << compatibility_for_f1[im] << std::endl;
                        }
                    }
                    
                }
            }            
        }    
        result[i] = has_conflict;
    }

    for (int i = 0; i < N; i++) {
        if (result[i] > 0) {
            return true;
        }
    }
    return false;
}