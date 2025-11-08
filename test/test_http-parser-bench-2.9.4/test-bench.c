#include "http_parser.h"
#include "unity.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

/* --- Test Data Definitions (Copied/Derived from test-http-parser.c) --- */

/* Scenario 1: Simple GET Request (Minimal) - requests[4] */
#define SCENARIO_SIMPLE_GET_RAW "GET /get_no_headers_no_body/world HTTP/1.1\r\n\r\n"
#define SCENARIO_SIMPLE_GET_LEN (sizeof(SCENARIO_SIMPLE_GET_RAW) - 1)

/* Scenario 2: Complex GET Request (Browser-like) - requests[1] */
#define SCENARIO_COMPLEX_GET_RAW "GET /favicon.ico HTTP/1.1\r\nHost: 0.0.0.0=5000\r\nUser-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: en-us,en;q=0.5\r\nAccept-Encoding: gzip,deflate\r\nAccept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\nKeep-Alive: 300\r\nConnection: keep-alive\r\n\r\n"
#define SCENARIO_COMPLEX_GET_LEN (sizeof(SCENARIO_COMPLEX_GET_RAW) - 1)

/* Scenario 3: POST Request with Content-Length (Small Body) - requests[7] */
#define SCENARIO_POST_CL_RAW "POST /post_identity_body_world?q=search#hey HTTP/1.1\r\nAccept: */*\r\nContent-Length: 5\r\n\r\nWorld"
#define SCENARIO_POST_CL_LEN (sizeof(SCENARIO_POST_CL_RAW) - 1)

/* Scenario 4: POST Request with Chunked Transfer-Encoding (Small Body) - requests[8] */
#define SCENARIO_POST_CHUNKED_RAW "POST /post_chunked_all_your_base HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n1e\r\nall your base are belong to us\r\n0\r\n\r\n"
#define SCENARIO_POST_CHUNKED_LEN (sizeof(SCENARIO_POST_CHUNKED_RAW) - 1)

/* Scenario 5: Simple 200 OK Response (Minimal) - responses[15] */
#define SCENARIO_SIMPLE_200_RESP_RAW "HTTP/1.1 200 OK\r\n\r\n"
#define SCENARIO_SIMPLE_200_RESP_LEN (sizeof(SCENARIO_SIMPLE_200_RESP_RAW) - 1)

/* --- Benchmarking Configuration --- */
/* Use a large number of iterations for meaningful results, but keep it reasonable for unit tests */
#define BENCH_ITERATIONS 100000
#define BYTES_PER_MB (1024.0 * 1024.0)

/* Dummy settings structure for performance testing (minimal callbacks) */
static http_parser_settings settings_perf = {
  .on_message_begin = NULL,
  .on_url = NULL,
  .on_header_field = NULL,
  .on_header_value = NULL,
  .on_headers_complete = NULL,
  .on_message_complete = NULL,
  .on_body = NULL,
  .on_status = NULL,
  .on_chunk_header = NULL,
  .on_chunk_complete = NULL
};

/* Helper function to get time in seconds (double) */
static double get_time_s() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

/* Generic function to run and report performance test */
void run_performance_test(const char* raw_data, size_t data_len, enum http_parser_type type, const char* scenario_name) {
    struct http_parser parser;
    int i;
    double start_time, end_time, elapsed_time;
    double total_bytes_processed;
    double mb_processed;
    double mb_per_sec;
    double req_per_sec;

    TEST_MESSAGE("--- Starting Performance Test: ");
    TEST_MESSAGE(scenario_name);
    TEST_MESSAGE(" ---");

    start_time = get_time_s();

    for (i = 0; i < BENCH_ITERATIONS; i++) {
        size_t parsed;
        http_parser_init(&parser, type);
        parsed = http_parser_execute(&parser, &settings_perf, raw_data, data_len);
        
        /* Assert successful parsing for consistent measurement */
        TEST_ASSERT_EQUAL_MESSAGE(data_len, parsed, "Parsing failed during benchmark iteration.");
    }

    end_time = get_time_s();
    elapsed_time = end_time - start_time;

    total_bytes_processed = (double)BENCH_ITERATIONS * data_len;
    mb_processed = total_bytes_processed / BYTES_PER_MB;
    
    if (elapsed_time > 0) {
        mb_per_sec = mb_processed / elapsed_time;
        req_per_sec = (double)BENCH_ITERATIONS / elapsed_time;
    } else {
        mb_per_sec = 0.0;
        req_per_sec = 0.0;
    }

    char report[512];
    snprintf(report, sizeof(report),
             "Scenario: %s\n"
             "  Iterations: %d\n"
             "  Data Size: %.2f bytes\n"
             "  Total Processed: %.2f MB\n"
             "  Elapsed Time: %.4f s\n"
             "  Throughput: %.2f MB/s\n"
             "  Rate: %.2f req/sec",
             scenario_name,
             BENCH_ITERATIONS,
             (double)data_len,
             mb_processed,
             elapsed_time,
             mb_per_sec,
             req_per_sec);
    
    TEST_MESSAGE(report);
    TEST_MESSAGE("--- Finished Performance Test: ");
    TEST_MESSAGE(scenario_name);
    TEST_MESSAGE(" ---\n");
}

/* --- Unity Test Cases --- */

void test_perf_scenario_1_simple_get(void) {
    run_performance_test(SCENARIO_SIMPLE_GET_RAW, SCENARIO_SIMPLE_GET_LEN, HTTP_REQUEST, "Simple GET Request (Minimal)");
}

void test_perf_scenario_2_complex_get(void) {
    run_performance_test(SCENARIO_COMPLEX_GET_RAW, SCENARIO_COMPLEX_GET_LEN, HTTP_REQUEST, "Complex GET Request (Browser-like)");
}

void test_perf_scenario_3_post_content_length(void) {
    run_performance_test(SCENARIO_POST_CL_RAW, SCENARIO_POST_CL_LEN, HTTP_REQUEST, "POST Request (Content-Length)");
}

void test_perf_scenario_4_post_chunked(void) {
    run_performance_test(SCENARIO_POST_CHUNKED_RAW, SCENARIO_POST_CHUNKED_LEN, HTTP_REQUEST, "POST Request (Chunked TE)");
}

void test_perf_scenario_5_simple_200_response(void) {
    run_performance_test(SCENARIO_SIMPLE_200_RESP_RAW, SCENARIO_SIMPLE_200_RESP_LEN, HTTP_RESPONSE, "Simple 200 OK Response");
}

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_perf_scenario_1_simple_get);
    RUN_TEST(test_perf_scenario_2_complex_get);
    RUN_TEST(test_perf_scenario_3_post_content_length);
    RUN_TEST(test_perf_scenario_4_post_chunked);
    RUN_TEST(test_perf_scenario_5_simple_200_response);
    return UNITY_END();
}


void app_main() {
    main();
}

