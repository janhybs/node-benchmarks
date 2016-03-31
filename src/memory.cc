#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <map>
#include <chrono>
#include <iostream>
#include <fstream>
#include "libs/json.hpp"

#define KB  1024
#define MB  1024 * 1024
#define REP 1000 * 1000
#define OFFSET 256
#define ARR_SIZE MB *100

using namespace std;
using json = nlohmann::json;

void print_result_debug(int size, long time) {
    printf("\rsize of %5d kB, ended with %10.3f ms", size/1024, time/1000.0);
    cout << flush;
}
void print_result(int size, long time) {
    printf("%5d, %10.3f\n", size/1024, time/1000.0);
}

template <int N, int M>
map<int, long> cpu_test_write(int (&arr)[M], int (&sizes)[N], int repetition = REP) {
    map<int, long> results;

    chrono::high_resolution_clock::time_point start, end;
    std::chrono::duration<double, std::micro> duration;
    int i, j, mod;
    const int i_max = sizeof(sizes)/sizeof(int);
    for (i = 0; i < i_max; i++) {
        mod = sizes[i] - 1;
        //-------------------------------------------------------
        start = std::chrono::high_resolution_clock::now();
            for (j = 0; j < repetition; j++) {
                arr[(j * OFFSET) & mod] = 0;
            }
        end = std::chrono::high_resolution_clock::now();
        //-------------------------------------------------------
        duration = std::chrono::duration_cast<chrono::nanoseconds>(end - start);
        results[sizes[i]] = duration.count();
        print_result_debug(sizes[i], results[sizes[i]]);
    }
    cout << "\rCPU write test ended                                 \n";
    
    return results;
}


template <int N, int M>
map<int, long> cpu_test_read(int (&arr)[M], int (&sizes)[N], int repetition = REP) {
    map<int, long> results;

    chrono::high_resolution_clock::time_point start, end;
    std::chrono::duration<double, std::micro> duration;
    int i, j, mod, sum;
    const int i_max = sizeof(sizes)/sizeof(int);
    for (i = 0; i < i_max; i++) {
        mod = sizes[i] - 1;
        //-------------------------------------------------------
        start = std::chrono::high_resolution_clock::now();
            for (j = 0; j < repetition; j++)  {
                sum += arr[(j * OFFSET) & mod];
            }
        end = std::chrono::high_resolution_clock::now();
        //-------------------------------------------------------
        duration = std::chrono::duration_cast<chrono::nanoseconds>(end - start);
        results[sizes[i]] = duration.count();
        print_result_debug(sizes[i], results[sizes[i]]);
    }
    cout << "\rCPU read test ended                                 " << sum << endl;
    
    return results;
}


template <int N, int M>
map<int, long> cpu_test_read_write_split(int (&arr)[M], int (&sizes)[N], int repetition = REP) {
    map<int, long> results;

    chrono::high_resolution_clock::time_point start, end;
    std::chrono::duration<double, std::micro> duration;
    int i, j, mod, sum;
    const int i_max = sizeof(sizes)/sizeof(int);
    for (i = 0; i < i_max; i++) {
        mod = sizes[i] - 1;
        //-------------------------------------------------------
        start = std::chrono::high_resolution_clock::now();
            for (j = 0; j < repetition; j++) {
                arr[(j * OFFSET) & mod] = 1;
            }
            for (j = 0; j < repetition; j++) {
                sum += arr[(j * OFFSET) & mod];
            }
        end = std::chrono::high_resolution_clock::now();
        //-------------------------------------------------------
        duration = std::chrono::duration_cast<chrono::nanoseconds>(end - start);
        results[sizes[i]] = duration.count();
        print_result_debug(sizes[i], results[sizes[i]]);
    }
    cout << "\rCPU r/w test ended                                 " << sum << endl;
    
    return results;
}

template <int N, int M>
map<int, long> cpu_test_read_write(int (&arr)[M], int (&sizes)[N], int repetition = REP) {
    map<int, long> results;

    chrono::high_resolution_clock::time_point start, end;
    std::chrono::duration<double, std::micro> duration;
    int i, j, mod, sum;
    const int i_max = sizeof(sizes)/sizeof(int);
    for (i = 0; i < i_max; i++) {
        mod = sizes[i] - 1;
        //-------------------------------------------------------
        start = std::chrono::high_resolution_clock::now();
            for (j = 0; j < repetition; j++) {
                arr[(j * OFFSET) & mod] *= 10;
                arr[(j * OFFSET) & mod] /= 10;
            }
        end = std::chrono::high_resolution_clock::now();
        //-------------------------------------------------------
        duration = std::chrono::duration_cast<chrono::nanoseconds>(end - start);
        results[sizes[i]] = duration.count();
        print_result_debug(sizes[i], results[sizes[i]]);
    }
    cout << "\rCPU r/w test ended                                 " << sum << endl;
    
    return results;
}



int main(int argc,  char* argv[]) {
    map<int, long> results_write, results_read, results_rw;
    static int arr[ARR_SIZE];
    float rep_cnt = argc >= 3 ? std::stof(argv[2]) : 1;
    
    static int sizes[] = {
        // 1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
        1 * KB, 2 * KB, 4 * KB, 8 * KB, 16 * KB, 32 * KB, 64 * KB, 128 * KB,
        256 * KB, 512 * KB, 1 * MB, 2 * MB, 4 * MB, 8 * MB, 16 * MB, 32 * MB
        // 4 * KB, 128 * KB, 8 * MB, 32 * MB
    };
    
    for (int i = 0; i < sizeof(arr)/sizeof(int); i++) {
        arr[i] = rand();
    }
    
    // results_read =  cpu_test_read(arr, sizes, REP * rep_cnt);
    // results_write = cpu_test_write(arr, sizes, REP * rep_cnt);
    results_rw =    cpu_test_read_write(arr, sizes, REP * rep_cnt);
    
    printf("%5s\t%5s\t%5s\t%5s\n", "size","read", "write", "r/w");
    
    
    // store additional information about experiment
    json j;
    j["reps"] = (int)(REP * rep_cnt);
    j["offset"] = OFFSET;
    j["arr_size"] = ARR_SIZE;
    for(int i = 0; i < sizeof(sizes)/sizeof(int); i++) {
        int size = (int)((sizes[i]/1024)*sizeof(int));
        printf("%d\t%1.3f\t%1.3f\t%1.3f\n", size, 
        results_read[sizes[i]]/1000.0,
        results_write[sizes[i]]/1000.0,
        results_rw[sizes[i]]/1000.0);
        
        j["rw"]["data"][to_string(size)] = results_rw[sizes[i]];
    }
    
    // write results to cout and json file
    cout << setw(4) << j << endl;
    if (argc >= 2) {
        ofstream ofs (argv[1]);
        ofs << setw(4) << j << endl;
    }
    
    return 0;
}