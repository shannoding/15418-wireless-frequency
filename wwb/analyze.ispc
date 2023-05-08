export enum IMProduct {
    im_chan, im_2T3O, im_2T5O, im_2T7O, im_2T9O, im_3T3O
};

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

export void quick_check_compatibility_arith(uniform const int N,
                            uniform const int num_im,
                            uniform const int (&frequencies)[], // size = N_F
                            uniform const int (&compatibility)[], // size = N_F * N_IM
                            uniform const IMProduct (&ims)[], // size = N_IM
                            uniform int result[])
{
    // print("hewwo");
    foreach (i = 0 ... N) {
        int f1 = frequencies[i];      
        const int *compatibility_for_f1 = &(compatibility[i]);
        // int* compatibility_for_f1 = new int[num_im];
        // memcpy((void*)&(compatibility[i * num_im]), (void*)compatibility_for_f1, num_im);

        int has_conflict = 0;

        // set result[i] = true if this frequency will experience a conflict
        // caused by someone else or an IM product

        for (uniform int j = 0; j < N; j++) {
            uniform int f2 = frequencies[j]; 

            if (i != j && freq_within_range(f1, f2, compatibility_for_f1[N * im_chan])) {
                // result[i] = true;
                has_conflict++;
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
                                // result[i] = true;
                                has_conflict++;

                            }
                        }
                    }
                    else {
                        int fc = check_im_product(f2, f3, 0, im);

                        if (freq_within_range(f1, fc, compatibility_for_f1[N * im])) {
                            // result[i] = true;
                            has_conflict++;
                        }
                    }
                    
                }   
            }
        } 
        result[i] = has_conflict;
    }
}

task void quick_check_compatibility_arith_task(uniform const int N,
                            uniform int span,
                            uniform const int num_im,
                            uniform const int (&frequencies)[], // size = N_F
                            uniform const int (&compatibility)[], // size = N_F * N_IM
                            uniform const IMProduct (&ims)[], // size = N_IM
                            uniform int result[])
{
    
    foreach (s = 0 ... span) {
        int i = taskIndex * span + s;
        int f1 = frequencies[i];      
        const int *compatibility_for_f1 = &(compatibility[i]);

        // result[i] = false; // assume no conflicts at the start
        int has_conflict = 0;

        // set result[i] = true if this frequency will experience a conflict
        // caused by someone else or an IM product

        for (int j = 0; j < N; j++) {
            int f2 = frequencies[j]; 

            if (i != j && freq_within_range(f1, f2, compatibility_for_f1[N * im_chan])) {
                // result[i] = true;
                has_conflict++;
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
                                // result[i] = true;
                                has_conflict++;

                            }
                        }
                    }
                    else {
                        int fc = check_im_product(f2, f3, 0, im);

                        if (freq_within_range(f1, fc, compatibility_for_f1[N * im])) {
                            // result[i] = true;
                            has_conflict++;
                        }
                    }
                    
                }   
            }
        } 
        result[i] = has_conflict;
    }
}


export void quick_check_compatibility_arith_withtasks(uniform const int N,
                            uniform const int num_im,
                            uniform const int (&frequencies)[], // size = N_F
                            uniform const int (&compatibility)[], // size = N_F * N_IM
                            uniform const IMProduct (&ims)[], // size = N_IM
                            uniform int result[]) {


    uniform int span = N / 8;  // N tasks

    launch[N/span] quick_check_compatibility_arith_task(
                N,
                span,
                num_im,
                frequencies, // size = N_F
                compatibility, // size = N_F * N_IM
                ims, // size = N_IM
                result
    );
                            }