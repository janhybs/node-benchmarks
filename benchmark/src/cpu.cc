#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>
#include <stdarg.h>
#include <functional>
#include <unistd.h>

#include "libs/json.hpp"
#include "libs/md5.hpp"

#define KB  1024
#define MB  1048576
#define REP (KB * KB)
#define OFFSET 1024
#define ARR_SIZE (100 * MB)

#define GIGA  1.0e+9
#define MEGA  1.0e+6
#define KILO  1.0e+3
#define MILI  1.0e-3
#define MICRO 1.0e-6
#define NANO  1.0e-9

#define CHAR_SIZE sizeof(char)
#define INT_SIZE  sizeof(int)

#define SHOW_DURATION true
#define SHOW_DETAILS  true

using namespace std;
using json = nlohmann::json;


static unsigned long x=123456789, y=362436069, z=521288629;

/**
 * Function will return pseudorandom value (long)
 * @return  pseudorandom value with period of 2^96-1
 */
unsigned long random_long(void) {          //period 2^96-1
    unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

   t = x;
   x = y;
   y = z;
   z = t ^ x ^ y;

  return z;
}


/**
 * Simple class for measuring time
 */
class Timer {
    protected:
        chrono::high_resolution_clock::time_point _start;
        chrono::high_resolution_clock::time_point _stop;
    public:
        chrono::duration<double, nano> duration;
        /**
         * Starts the timer
         */
        void start() {
            this->_start = std::chrono::high_resolution_clock::now();
        }
        /**
         * Stops the timer and calculates duration of the measured block
         */
        void stop() {
            this->_stop = std::chrono::high_resolution_clock::now();
            this->duration = chrono::duration_cast<chrono::nanoseconds>(this->_stop - this->_start);
        }
};


/**
 * Function will return pseudorandom string using string generator
 * Returned string is consisting of A-Za-z0-9
 * @param  length length of the string
 * @return        A-Za-z0-9 string of given length
 */
std::string random_string( size_t length ) {
    auto randchar = []() -> char {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ random_long() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}


/**
 * Function prints message (with printf format support) 
 * with \r carriage set (output will remain on the same line)
 * @param fmt     format prescription
 * @param VARARGS additional arguments passed to format function
 */
void printf_debug(const char * fmt, ...) {
    printf("                                          \r");
    string newfmt = string(fmt) + "\r";
    const char * newfmt_c = newfmt.c_str();
    va_list args;
    va_start(args, fmt);
    vprintf(newfmt_c, args);
    va_end(args);
    cout << flush;
}


/**
 * TEST will sum random longs for the given amount of reps
 * total number of is also given by global REP value
 * total = reps*REP
 *
 * This test is testing CPU speed, there should not be
 * any memory stress since we are only using single variable
 * All other routines such as random generation should fit into L1 cache
 *
 * Following json dictionary is merged with given json:
 *    {
 *      duration: <duration in seconds>,
 *      size:     <total number of reps>
 *    }
 * 
 * @param results json holder
 * @param reps    number of reps
 */
void test_reg_simple(json &results, int reps = 512) {
    printf_debug("cpu simple, size=%d, reps=%d", REP, reps);
    Timer timer;
    
    //-------------------------------------------------------
    timer.start();
    int sum;
    for (size_t j = 0; j < reps; j++) {
        for (size_t i = 0; i < REP; i++) {
            sum += random_long();
        }
    }
    timer.stop();
    //-------------------------------------------------------
    results["duration"] = timer.duration.count() * NANO;
    results["size"]     = reps * REP;
}


/**
 * TEST will generate random longs and will use simple build-in hash function
 * storing it in the single variable (no append, value is set - memory consumption is constant)
 * total number of is also given by global REP value
 * total = reps*REP
 *
 * This test is testing CPU speed, there should not be
 * any memory stress since we are only using single variable
 * All other routines such as random generation should fit into L1 cache
 *
 * Following json dictionary is merged with given json:
 *    {
 *      duration: <duration in seconds>,
 *      size:     <total number of reps>
 *    }
 * 
 * @param results json holder
 * @param reps    number of reps
 */
void test_reg_hash(json &results, int reps = 128) {
    printf_debug("cpu hash, size=%d, reps=%d", REP, reps);
    Timer timer;
    hash<int> int_hash;
    
    //-------------------------------------------------------
    timer.start();
    string res;
    const int HALF = RAND_MAX / 2;
    for (size_t j = 0; j < reps; j++) {
        for (size_t i = 0; i < REP; i++) {
            res = int_hash(random_long() - HALF);
        }
    }
    timer.stop();
    //-------------------------------------------------------
    results["duration"] = timer.duration.count() * NANO;
    results["size"]     = reps * REP;
}


/**
 * TEST will generate random string of length 16 y default and will use md5 hash on result
 * storing it in the single variable (no append, value is set - memory consumption is constant)
 * total number of is also given by global REP value
 * total = reps*REP
 *
 * This test is testing CPU speed, there should not be
 * any memory stress since we are only using single variable
 * All other routines such as random generation should fit into L1-l2 cache
 *
 * Following json dictionary is merged with given json:
 *    {
 *      duration: <duration in seconds>,
 *      size:     <total number of reps>
 *    }
 * 
 * @param results json holder
 * @param reps    number of reps
 * @param str_len length of the random string
 */
void test_reg_md5(json &results, int reps = 1, int str_len = 16) {
    printf_debug("cpu md5, size=%d, reps=%d", REP, reps);
    Timer timer;
    
    //-------------------------------------------------------
    timer.start();
    string res;
    for (size_t j = 0; j < reps; j++) {
        for (size_t i = 0; i < REP; i++) {
            res = md5(random_string(str_len));
        }
    }
    timer.stop();
    //-------------------------------------------------------
    results["duration"] = timer.duration.count() * NANO;
    results["size"]     = reps * REP * str_len;
}


/**
 * Test will read and write values on array elements. R/W order is given by 
 * value sizes, where lower number restricts access of the array elements
 * meaning lower number will force lower cache to work
 *
 * This test is testing MEMORY speed. It can stress all types of memory 
 * from L1 to RAM based on given sizes value.
 * 
 * Following json dictionary is merged with given json:
 *    {
 *      duration: <duration in seconds>,
 *      size:     <total number of reps>
 *      N:        <number of sizes in test>
 *    }
 */
template <int N>
int* test_mem(json &results, int (&sizes)[N], int size = ARR_SIZE, int reps=32) {
    printf_debug("creating array...         ");
    static int arr[ARR_SIZE];
    int sum = 0;
    int mod = 16-1;
    Timer timer;
    printf_debug("mem, size=%d, reps=%d", REP * reps, N);
    
    //-------------------------------------------------------
    timer.start();
    for (int i = 0; i < N; i++) {
        printf_debug("mem, size=%d, reps=%d, (%02d/%02d)", REP * reps, N, i+1, N);
        mod = sizes[i] - 1;

        for (int j = 0; j < REP * reps; j++) {
            arr[(j * OFFSET) & mod]++;
            // arr[(j * OFFSET) & mod]--;
            // sum += arr[(j * OFFSET) & mod];
            // arr[(i * MB + i) & ARR_SIZE] = i;//random_long();
        }
        // results["duration_" + to_string(sizes[i])] = timer.duration.count() * NANO;
    }
    timer.stop();
    //-------------------------------------------------------
    
    results["duration"] = timer.duration.count() * NANO;
    results["size"]     = REP * N * reps;
    results["n"]        = N;
    return arr;
}


/**
 * Test will read and write values on array elements. R/W order is given by 
 * value sizes, where lower number restricts access of the array elements
 * meaning lower number will force lower cache to work
 *
 * This test is testing MEMORY speed. It can stress all types of memory 
 * from L1 to RAM based on given sizes value.
 * 
 * Following json dictionary is merged with given json:
 *    {
 *      duration: <duration in seconds>,
 *      size:     <total number of reps>
 *    }
 */
void mat_mul(json &results, int size=256, int reps=16) {
    int aMatrix[size][size];
    int bMatrix[size][size];
    int product[size][size];
    Timer timer;
    printf_debug("generating matrices, size=%dx%d", size, size);
    
    // first create matrices
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            aMatrix[i][j] = random_long();
            bMatrix[i][j] = random_long();
            product[i][j] = 0;
        }
    }
    
    printf_debug("mat mul, size=%dx%d, reps=%d", size, size, reps);
    
    //-------------------------------------------------------
    timer.start();
    // do the multiplication
    for (size_t k = 0; k < reps; k++) {
        for (int row = 0; row < size; row++) {
            for (int col = 0; col < size; col++) {
                // Multiply the row of A by the column of B to get the row, column of product.
                for (int inner = 0; inner < size; inner++) {
                    product[row][col] += aMatrix[row][inner] * bMatrix[inner][col];
                }
            }
        }
    }
    timer.stop();
    //-------------------------------------------------------
    
    results["duration"] = timer.duration.count() * NANO;
    results["size"]     = reps * size * size * size;
}



/**
 * Start benchmark, usage:
 * optional <output> file: will create json file output with results
 * optional <scale> float: scales repetition count for benchmark
 *                         default is 1, for example value 2 will run tests 
 *                         twice as many times, value 0.5 will experiments half
 *                         as many times
 */
int main(int argc,  char* argv[]) {
    int rep_cnt = (int)(argc >= 3 ? std::stof(argv[2]) * REP : 1 * REP);
    
    printf_debug("running tests...         ");
    json results;
    results["version"] = "1.0.1";
    
    
    Timer test_timer;
    test_timer.start();
    // ------------------------------------------------------------------------
    
    // valgrind: D1   miss rate  0.0%
    // valgrind: LLd  miss rate  0.0%
    // valgrind: LL   miss rate  0.0%
    int sizes_l1[] = { 4, 8, 16, 32, 64, 128, 256, 512, 1 * KB, 2 * KB };
    test_mem(results["mem_l1"], sizes_l1);
    // 
    // // valgrind: D1   miss rate  9.5%
    // // valgrind: LLd  miss rate  0.0%
    // // valgrind: LL   miss rate  0.0%
    int sizes_l2[] = { 4 * KB, 8 * KB, 16 * KB, 32 * KB, 64 * KB, 128 * KB };
    test_mem(results["mem_l2"], sizes_l2);
    
    // valgrind: D1   miss rate 14.2%
    // valgrind: LLd  miss rate  5.7%
    // // valgrind: LL   miss rate  1.9%
    int sizes_l3[] = { 256 * KB, 512 * KB, 1 * MB, 2 * MB, 4 * MB };
    test_mem(results["mem_l3"], sizes_l3);
    // 
    // // valgrind: D1   miss rate 14.2%
    // // valgrind: LLd  miss rate 14.2%
    // // valgrind: LL   miss rate  4.9%
    int sizes_ll[] = { 8 * MB, 16 * MB, 32 * MB };
    test_mem(results["mem_ll"], sizes_ll);
    
    // valgrind: D1   miss rate  0.0%
    // valgrind: LLd  miss rate  0.0%
    // valgrind: LL   miss rate  0.0%
    test_reg_simple(results["cpu_simple"]);
    
    // valgrind: D1   miss rate  0.0%
    // valgrind: LLd  miss rate  0.0%
    // valgrind: LL   miss rate  0.0%
    test_reg_hash  (results["cpu_hash"]);
    
    // valgrind: D1   miss rate  0.0%
    // valgrind: LLd  miss rate  0.0%
    // valgrind: LL   miss rate  0.0%
    test_reg_md5   (results["cpu_md5"]);
    
    
    
    mat_mul(results["mat_mul_s1"],  16, 4*8*8*8*8*8);
    mat_mul(results["mat_mul_s2"],  64, 4*8*8*8);
    mat_mul(results["mat_mul_s3"], 128, 4*8*8);
    mat_mul(results["mat_mul_s4"], 512, 4);
    // ------------------------------------------------------------------------
    test_timer.stop();
    printf("---------------------------------\n");
    
    printf("%-30s: %1.3f\n", "time taken", test_timer.duration.count() * NANO);
    
    printf_debug("generating output...    \n");
    cout << results.dump(true) << endl;

    // if arg is set we redirect dump to file
    if (argc >= 2) {
        ofstream ofs (argv[1]);
        ofs << results.dump(true) << endl;
    }
    
    return 0;
}
