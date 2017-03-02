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
#include "libs/SparseMatrix/SparseMatrix.cpp"

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
unsigned long random_long(void) {
   //  unsigned long t;
   //  x ^= x << 16;
   //  x ^= x >> 5;
   //  x ^= x << 1;
   // 
   // t = x;
   // x = y;
   // y = z;
   // z = t ^ x ^ y;
   // 
   // return z;
   return rand();
}


/**
 * Function will return random value between min and max
 * @param  min min value inclusive
 * @param  max max value inclusive
 * @return     <min, max>
 */
int random_range(int min=1, int max=9) {
    return min + (rand() % (int)(max - min + 1));
}


/**
 * Generates vector of given lenght where evry value is unique
 */
vector<int> generate_random_coords(int length, int min, int max) {
    vector<int> v(length, 0);
    int val;
    for (size_t i = 0; i < length; i++) {
        do {
            val = random_range(min, max - 1);
        } while(std::find(v.begin(), v.end(), val) != v.end());
        v[i] = val;
    }
    
    std::sort (v.begin(), v.end());
    return v;
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
 * Will generate random matrix rows x cols and will insert per_line random elements
 * on single line, position is of these elements is random around diagonal with band spreading
 * @param  rows
 * @param  cols
 * @param  per_line
 * @param  band     rande of the element on the line is diagonal +- band
 * @return
 */
SparseMatrix<int> generate_random_sparse_matrix(int rows, int cols, int per_line, int band) {
    SparseMatrix<int> mat(rows, cols);
    vector<int> coords;
    int left, right;
    for (size_t i = 0; i < rows; i++) {
        int middle = ((float)i / rows) * cols;
        left = middle - band;
        right = middle  + band;

        if (band >= cols/2) {
            left = 0;
            right = cols;
        } else {
            if (left < 0) {
                right += abs(left);
                left = 0;
            }

            if (right >= cols) {
                left += cols - right;
                right = cols;
            }
        }
        
        coords = generate_random_coords(per_line, left, right);
        for (size_t j = 0; j < per_line; j++) {
            mat.set(
                random_long(),      // random value
                i+1,                // random x coordinate (indexing from 1)
                coords[j]+1         // random y coordinate (indexing from 1)
            );
        }
    }
    return mat;
}

/**
 * Will generate new random vector of given lenght
 * @param  length
 * @return
 */
vector<int> generate_random_vector(int length) {
    vector<int> vec(length, 0);
    for (size_t i = 0; i < length; i++) {
        vec[i] = random_long();
    }
    return vec;
}

/**
 * Will print given matrix python style
 * @param mat
 */
void print_matrix_python(SparseMatrix<int> mat) {
    int rows = mat.getRowCount();
    int cols = mat.getColumnCount();

    std::cout << "[" << std::endl;
    for (size_t i = 0; i < rows; i++) {
        std::cout << "[";
        for (size_t j = 0; j < cols; j++) {
            std::cout << mat.get(i + 1, j + 1) << ", ";
        }
        std::cout << "]," << std::endl;
    }
    std::cout << "]" << std::endl;
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
    results["size"]     = REP;
    results["reps"]     = reps;
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
    results["size"]     = REP;
    results["reps"]     = reps;
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
    results["size"]     = REP * str_len;
    results["reps"]     = reps;
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

        for (long j = 0; j < REP * reps; j++) {
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
    results["size"]     = N;
    results["reps"]     = reps;
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
    int k, row, col, inner;
    timer.start();
    // do the multiplication
    for (k = 0; k < reps; k++) {
        for (row = 0; row < size; row++) {
            for (col = 0; col < size; col++) {
                // Multiply the row of A by the column of B to get the row, column of product.
                for (inner = 0; inner < size; inner++) {
                    product[row][col] += aMatrix[row][inner] * bMatrix[inner][col];
                }
            }
        }
    }
    timer.stop();
    //-------------------------------------------------------
    
    results["duration"] = timer.duration.count() * NANO;
    results["size"]     = size;
    results["reps"]     = reps;
}


/**
 * Test will generate random matrix rows x cols, and 
 * vector of length cols and populate both structures with data (for matrix 
 * total of n data is generated).
 * We then perform matrix vector multiplication (format CSR for sparse matrices)
 *
 * This test is testing MEMORY speed and also CPU speed. Memory access is random-like
 * 
 * 
 * Following json dictionary is merged with given json:
 *    {
 *      duration: <duration in seconds>,
 *      size:     <total number of operations>
 *    }
 */
void test_sparse_mat_vec(json &results, int rows=100, int cols=100, int per_line=10, int band=10, int reps=512) {
    printf_debug("generating matrix, size=%dx%d, per_line=%d, band=%d           ",
        rows, cols, per_line, band);
    SparseMatrix<int> mat = generate_random_sparse_matrix(rows, cols, per_line, band);
    
    printf_debug("generating vector, size=%dx%d, per_line=%d, band=%d           ",
        rows, cols, per_line, band);
    vector<int> vec = generate_random_vector(cols);
    
    Timer timer;
    printf_debug("sparse mat vec mul, size=%dx%d, per_line=%d, band=%d          ",
        rows, cols, per_line, band);
    
    //-------------------------------------------------------
    int i;
    timer.start();
    for (i = 0; i < reps; i++) {
        vector<int> result;
        result = mat * vec;
    }
    timer.stop();
    //-------------------------------------------------------
    
    results["duration"] = timer.duration.count() * NANO;
    results["ops"]      = rows * per_line * cols;
    results["rows"]     = rows;
    results["cols"]     = cols;
    results["reps"]     = reps;
    results["band"]     = band;
    results["per_line"] = per_line;
}


/**
 * Test will generate random matrix rows x cols, and 
 * vector of length cols and populate both structures with data (for matrix 
 * total of n data is generated).
 * We then perform matrix vector multiplication (format CSR for sparse matrices)
 *
 * This test is testing MEMORY speed and also CPU speed. Memory access is random-like
 * 
 * 
 * Following json dictionary is merged with given json:
 *    {
 *      duration: <duration in seconds>,
 *      size:     <total number of operations>
 *    }
 */
void test_sparse_mat_mat(json &results, int rows=100, int cols=100, int per_line=10, int band=10, int reps=512) {
    printf_debug("generating matrices, size=%dx%d, per_line=%d, band=%d         ",
        rows, cols, per_line, band);
    SparseMatrix<int> A = generate_random_sparse_matrix(rows, cols, per_line, band);
    SparseMatrix<int> B = generate_random_sparse_matrix(cols, rows, per_line, band);
    Timer timer;
    
    printf_debug("sparse mat mat mul, size=%dx%d, per_line=%d, band=%d          ",
        rows, cols, per_line, band);
    double time_total = 0;
    
    
    
    //-------------------------------------------------------
    int i;
    timer.start();
    for (i = 0; i < reps; i++) {
        SparseMatrix<int> C = A * B;
    }
    timer.stop();
    //-------------------------------------------------------
    
    
    results["duration"] = timer.duration.count() * NANO;
    results["ops"]      = rows * per_line * cols;
    results["rows"]     = rows;
    results["cols"]     = cols;
    results["reps"]     = reps;
    results["band"]     = band;
    results["per_line"] = per_line;
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
    int i = 1;
    string json_file        = argc >= (i+1) ? string(argv[i++]) : "result.json";
    string version          = argc >= (i+1) ? string(argv[i++]) : "1.2.1";
    int el_per_line_extra   = argc >= (i+1) ? std::stof(argv[i++]) : 0;
    int line_spread_extra   = argc >= (i+1) ? std::stof(argv[i++]) : 0;

    int pl = el_per_line_extra;
    int sp = line_spread_extra;

    srand(1234);
    
    printf_debug("running tests...         ");
    json results;
    results["version"] = version;
    
    
    Timer test_timer;
    test_timer.start();
    // ------------------------------------------------------------------------
    
    // valgrind: D1   miss rate  0.0%
    // valgrind: LLd  miss rate  0.0%
    // valgrind: LL   miss rate  0.0%
    int sizes_l1[] = { 4, 8, 16, 32, 64, 128, 256, 512, 1 * KB, 2 * KB };
    test_mem(results["mem_l1"], sizes_l1, ARR_SIZE, 32*2);
    // 
    // // valgrind: D1   miss rate  9.5%
    // // valgrind: LLd  miss rate  0.0%
    // // valgrind: LL   miss rate  0.0%
    int sizes_l2[] = { 4 * KB, 8 * KB, 16 * KB, 32 * KB, 64 * KB, 128 * KB };
    test_mem(results["mem_l2"], sizes_l2, ARR_SIZE, 32*2);
    
    // valgrind: D1   miss rate 14.2%
    // valgrind: LLd  miss rate  5.7%
    // // valgrind: LL   miss rate  1.9%
    int sizes_l3[] = { 256 * KB, 512 * KB, 1 * MB, 2 * MB, 4 * MB };
    test_mem(results["mem_l3"], sizes_l3, ARR_SIZE, 32*2);
    // 
    // // valgrind: D1   miss rate 14.2%
    // // valgrind: LLd  miss rate 14.2%
    // // valgrind: LL   miss rate  4.9%
    int sizes_ll[] = { 8 * MB, 16 * MB, 32 * MB };
    test_mem(results["mem_ll"], sizes_ll, ARR_SIZE, 32*2);
    
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
    
    
    mat_mul(results["mmn_s1"],  16, 8*8*8*8*8*8);
    mat_mul(results["mmn_s2"],  64, 8*8*8*8);
    mat_mul(results["mmn_s3"], 128, 8*8*8);
    mat_mul(results["mmn_s4"], 256, 8*8);
    
    //                                     rows     cols    per_line    band            reps
    test_sparse_mat_vec(results["mvs_s1"], 32,      64,     20,         50,             1000*32*32);
    test_sparse_mat_vec(results["mvs_s2"], 128,     1024,   20,         50,             1000*16*16);
    test_sparse_mat_vec(results["mvs_s3"], 1024,    8192,   50+pl,      50+sp,          1000*10);
    test_sparse_mat_vec(results["mvs_s4"], 8192,    8192*4, 50,         50,             1000*2);
    //                                     rows     cols    per_line    band            reps
    test_sparse_mat_mat(results["mms_s1"], 8,       8,      2,          4,              1000*32*4);
    test_sparse_mat_mat(results["mms_s2"], 32,      32,     4,          8,              100*16);
    test_sparse_mat_mat(results["mms_s3"], 128,     128,    10,         20,             16);
    test_sparse_mat_mat(results["mms_s4"], 256,     256,    10,         20,             3);
    // ------------------------------------------------------------------------
    test_timer.stop();
    printf("--------------------------------------------------------------------\n");
    
    
    printf("%-30s: %1.3f\n", "time taken", test_timer.duration.count() * NANO);
    
    printf_debug("generating output...    \n");
    cout << results.dump(true) << endl;
    cout << json_file << endl;

    // if arg is set we redirect dump to file
    if (!json_file.empty()) {
        ofstream ofs (json_file);
        ofs << results.dump(true) << endl;
    }
    
    return 0;
}
