#pragma once

#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <time.h>

void set_core(int core, char *print_info);

double time_diff_ms(struct timespec begin, struct timespec end);
int comp(const void * a, const void * b);
int comp_double(const void * a, const void * b);

int median(int *array, int len);

void construct_L1_eviction_set(uint64_t* evset, uint64_t* page, uint64_t target, uint8_t SIZE, uint64_t STRIDE, uint64_t MAX_RAND);

void configure_thresholds(uint64_t target_addr, int* thrL1, int* thrRAM, int* thrDET);
int ground_truth(uint64_t* a, uint64_t* evset, uint8_t LLC_ASSOCIATIVITY, uint64_t thrDET);