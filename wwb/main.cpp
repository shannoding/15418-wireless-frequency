#include "main.h"



int main(int argc, char **argv) {
    std::fstream new_file;

    char *f_name = argv[2];
    std::cout << f_name << std::endl;
    
    
    new_file.open(f_name, std::fstream::in); 

    int N = atoi(argv[1]);
    int frequencies[N];
    int compatibility[N * NUM_OF_IM_PRODUCTS];

    int band_low[N];
    int band_high[N];
    
    // Checking whether the file is open.
    if (new_file.is_open()) { 
        std::string sa;
        // Read data from the file object and put it into a string.
        int line_num = 0;
        while (getline(new_file, sa)) {
            if (line_num < N) {
                frequencies[line_num] = std::stoi(sa);
            }
            else if (line_num < 2 * N) {
                std::stringstream ssin(sa);
                std::string intermediate;
     
                // Tokenizing w.r.t. space ' '
                for (int i = 0; i < NUM_OF_IM_PRODUCTS; i++)
                {
                    ssin >> intermediate;
                    compatibility[i * N + (line_num - N)] = std::stoi(intermediate);
                }
            }
            else if (line_num < 3 * N) {
                std::stringstream ssin(sa);
                std::string intermediate;

                ssin >> intermediate;
                band_low[(line_num - 2 * N)] = std::stoi(intermediate);

                ssin >> intermediate;
                band_high[(line_num - 2 * N)] = std::stoi(intermediate);
            }
            line_num++;
        }
        
        // Close the file object.
        new_file.close(); 
    }
    

    int result[N];
    for (int i = 0; i < N; i++) {
        result[i] = 0;
    }
    IMProduct ims[NUM_OF_IM_PRODUCTS] = { im_chan, im_2T3O, im_2T5O, im_2T7O, im_2T9O, im_3T3O };

    double minSerial = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime =CycleTimer::currentSeconds();
        bool is_compatible = quick_check_compatibility_arith_serial(
                            N,
                            NUM_OF_IM_PRODUCTS,
                            frequencies, // size = N_F
                            compatibility, // size = N_F * N_IM
                            ims, // size = N_IM
                            result);
    
        std::cout << is_compatible << std::endl;
        double endTime = CycleTimer::currentSeconds();
        minSerial = std::min(minSerial, endTime - startTime);
    }

    printf("[compatibility serial]:\t\t[%.3f] ms\n",
           minSerial * 1000);
    std::cout << "bad freqs: ";
    for (int i = 0; i < N; i++) {
        std::cout << i << "=" << result[i] << ",";
    }
    std::cout << std::endl;

    for (int i = 0; i < N; i++) {
        result[i] = 0;
    }

    //
    // Run the ISPC (single core) implementation
    //
    double minISPC = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        ispc::quick_check_compatibility_arith(
                N,
                NUM_OF_IM_PRODUCTS,
                frequencies, // size = N_F
                compatibility, // size = N_F * N_IM
                ims, // size = N_IM
                result);
        double endTime = CycleTimer::currentSeconds();
        minISPC = std::min(minISPC, endTime - startTime);
    }

    printf("[analyze ispc]:\t\t[%.3f] ms\n",
           minISPC * 1000);
    
    std::cout << "bad freqs: ";
    for (int i = 0; i < N; i++) {
        std::cout << i << "=" << result[i] << ",";
    }
    std::cout << std::endl;

    for (int i = 0; i < N; i++) {
        result[i] = -1;
    }

    //
    // Run the ISPC (multi core) implementation
    //
    double minTaskISPC = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        ispc::quick_check_compatibility_arith_withtasks(
                N,
                NUM_OF_IM_PRODUCTS,
                frequencies, // size = N_F
                compatibility, // size = N_F * N_IM
                ims, // size = N_IM
                result);
        double endTime = CycleTimer::currentSeconds();
        minTaskISPC = std::min(minTaskISPC, endTime - startTime);
    }

    printf("[analyze task ispc]:\t\t[%.3f] ms\n",
           minTaskISPC * 1000);
    
    std::cout << "bad freqs: ";
    for (int i = 0; i < N; i++) {
        std::cout << i << "=" << result[i] << ",";
    }
    std::cout << std::endl;

    for (int i = 0; i < N; i++) {
        result[i] = 0;
    }
    
    printf("\t\t\t\t(%.2fx speedup from ISPC)\n", minSerial/minISPC);
    printf("\t\t\t\t(%.2fx speedup from task ISPC)\n", minSerial/minTaskISPC);

    
    
    //
    // Run the serial implementation
    //
    double minOptimize = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        int n = calculate_frequencies_serial(N, 
                band_low, 
                band_high,
                frequencies,
                compatibility
                );
        double endTime = CycleTimer::currentSeconds();
        minOptimize = std::min(minOptimize, endTime - startTime);
        
        std::cout << "fit " << n << " frequencies" << std::endl;
        for (int i = 0; i < n; i++) {
            std::cout << frequencies[i] << " ";
        }
        std::cout << std::endl;
    }

    printf("[optimize serial]:\t\t[%.3f] ms\n",
           minOptimize * 1000);
    

    //
    // Run the ISPC (single core) implementation
    //
    double minOptimizeISPC = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        int n = calculate_frequencies_ispc_wrapper(N, 
                band_low, 
                band_high,
                frequencies,
                compatibility
                );

        double endTime = CycleTimer::currentSeconds();
        minOptimizeISPC = std::min(minOptimizeISPC, endTime - startTime);
        std::cout << "fit " << n << " frequencies" << std::endl;
        for (int i = 0; i < n; i++) {
            std::cout << frequencies[i] << " ";
        }
        std::cout << std::endl;
    }

    printf("[optimize ISPC]:\t\t[%.3f] ms\n",
           minOptimizeISPC * 1000);
    
    /*
    bool is_compatible1 = quick_check_compatibility_arith_serial(
                            N,
                            NUM_OF_IM_PRODUCTS,
                            frequencies, // size = N_F
                            compatibility, // size = N_F * N_IM
                            ims, // size = N_IM
                            result);

    std::cout << is_compatible1 << std::endl;
    std::cout << "bad freqs: ";
    for (int i = 0; i < N; i++) {
        std::cout << i << "=" << result[i] << ",";
    }
    std::cout << std::endl;
    */

    //
    // Run the ISPC (single core) implementation
    //
    double minOptimizeTaskISPC = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        int n = calculate_frequencies_ispc_tasks_wrapper(N, 
                band_low, 
                band_high,
                frequencies,
                compatibility
                );

        double endTime = CycleTimer::currentSeconds();
        minOptimizeTaskISPC = std::min(minOptimizeTaskISPC, endTime - startTime);
        std::cout << "fit " << n << " frequencies" << std::endl;
        for (int i = 0; i < n; i++) {
            std::cout << frequencies[i] << " ";
        }
        std::cout << std::endl;
    }

    printf("[optimize task ISPC]:\t\t[%.3f] ms\n",
           minOptimizeTaskISPC * 1000);

    printf("\t\t\t\t(%.2fx speedup from ISPC)\n", minOptimize/minOptimizeISPC);
    printf("\t\t\t\t(%.2fx speedup from task ISPC)\n", minOptimize/minOptimizeTaskISPC);

    return 0;

   
}
