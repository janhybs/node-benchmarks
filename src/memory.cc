#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <vector>
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

void cpu_test(json &results, int repetition = REP) {
    chrono::high_resolution_clock::time_point start, end;
    chrono::duration<double, micro> duration;
    int i, sum;
    //-------------------------------------------------------
    start = std::chrono::high_resolution_clock::now();
        for (i = 0; i < repetition; i++) {
            sum += i;
        }
    end = std::chrono::high_resolution_clock::now();
    //-------------------------------------------------------
    duration = chrono::duration_cast<chrono::microseconds>(end - start);
    results["duration"] = duration.count();
    results["reps"] = repetition;
}


template <int N, int M>
void cpu_test_r(json &results, int (&arr)[M], int (&sizes)[N], int repetition = REP) {
    chrono::high_resolution_clock::time_point start, end;
    chrono::duration<double, micro> duration;
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
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        results["duration"][to_string(sizes[i]/1024*sizeof(int))] = duration.count();
    }
    results["reps"] = repetition;
    results["size"] = sizeof(arr)/sizeof(int);
    results["sum"] = sum;
}


template <int N, int M>
void cpu_test_w(json &results, int (&arr)[M], int (&sizes)[N], int repetition = REP) {
    chrono::high_resolution_clock::time_point start, end;
    chrono::duration<double, micro> duration;
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
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        results["duration"][to_string(sizes[i]/1024*sizeof(int))] = duration.count();
    }
    results["reps"] = repetition;
    results["size"] = sizeof(arr)/sizeof(int);
}


template <int N, int M>
void cpu_test_rw(json &results, int (&arr)[M], int (&sizes)[N], int repetition = REP) {
    chrono::high_resolution_clock::time_point start, end;
    chrono::duration<double, micro> duration;
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
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        results["duration"][to_string(sizes[i]/1024*sizeof(int))] = duration.count();
    }
    results["reps"] = repetition;
    results["size"] = sizeof(arr)/sizeof(int);
    results["sum"] = sum;
}

template <int N, int M>
void cpu_test_read_write_split(json &results, int (&arr)[M], int (&sizes)[N], int repetition = REP) {
    chrono::high_resolution_clock::time_point start, end;
    chrono::duration<double, micro> duration;
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
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        results["duration"][to_string(sizes[i]/1024*sizeof(int))] = duration.count();
    }
    results["reps"] = repetition;
    results["size"] = sizeof(arr)/sizeof(int);
    results["sum"] = sum;
}


int main(int argc,  char* argv[]) {
    map<int, long> results_write, results_read, results_rw, results_cpu;
    static int arr[ARR_SIZE];
    int rep_cnt = (int)(argc >= 3 ? std::stof(argv[2]) * REP : 1 * REP);
    
    // chunk size for testing
    static int sizes[] = {
        // 1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
        1 * KB, 2 * KB, 4 * KB, 8 * KB, 16 * KB, 32 * KB, 64 * KB, 128 * KB,
        256 * KB, 512 * KB, 1 * MB, 2 * MB, 4 * MB, 8 * MB, 16 * MB, 32 * MB
        // 4 * KB, 128 * KB, 8 * MB, 32 * MB
    };
    
    // randomize array
    for (int i = 0; i < sizeof(arr)/sizeof(int); i++) {
        arr[i] = rand();
    }
    
    json results;
    
    cpu_test    (results["cpu"], rep_cnt);
    cpu_test_r  (results["r"], arr, sizes, rep_cnt);
    cpu_test_w  (results["w"], arr, sizes, rep_cnt);
    cpu_test_rw (results["rw"], arr, sizes, rep_cnt);
    
    cout << results.dump(true) << endl;
    if (argc >= 2) {
        ofstream ofs (argv[1]);
        ofs << results.dump(true) << endl;
    }
    // results_read =  cpu_test_read(arr, sizes, REP * rep_cnt);
    // results_write = cpu_test_write(arr, sizes, REP * rep_cnt);
    // results_rw =    cpu_test_read_write(arr, sizes, REP * rep_cnt);
    // results_cpu = cpu_test(REP * rep_cnt);
    
    // printf("%5s\t%5s\t%5s\t%5s\n", "size","read", "write", "r/w");
    
    
    // store additional information about experiment
    // j["reps"] = (int)(REP * rep_cnt);
    // j["offset"] = OFFSET;
    // j["arr_size"] = ARR_SIZE;
    // for(int i = 0; i < sizeof(sizes)/sizeof(int); i++) {
    //     int size = (int)((sizes[i]/1024)*sizeof(int));
    //     printf("%d\t%1.3f\t%1.3f\t%1.3f\n", size, 
    //     results_read[sizes[i]]/1000.0,
    //     results_write[sizes[i]]/1000.0,
    //     results_rw[sizes[i]]/1000.0);
    //     
    //     j["rw"]["data"][to_string(size)] = results_rw[sizes[i]];
    // }
    
    // write results to cout and json file

    
    return 0;
}