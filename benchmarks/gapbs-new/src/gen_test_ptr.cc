// user defined headers
#include <pf_interface.h>
#include <sim_api.h>

// STL headers
#include <cassert>
#include <cstdint>
#include <iostream>

// helper function for initialization
void
initialize(uint64_t** one, uint64_t* two, const int one_size, const int two_size)
{
    assert(one_size == 17);
    assert(two_size > 255);

    // this doesn't really matter cuz we don't traverse farther than this
    for (int i = 0; i < two_size; ++i) {
        two[i] = (uint64_t) i;
    }

    // hardcode in sequence to make sure debugging is easier
    one[0]  = two + 0;
    one[1]  = two + 5;
    one[2]  = two + 6;
    one[3]  = two + 35;
    one[4]  = two + 100;
    one[5]  = two + 112;
    one[6]  = two + 113;
    one[7]  = two + 113;
    one[8]  = two + 117;
    one[9]  = two + 180;
    one[10] = two + 200;
    one[11] = two + 205;
    one[12] = two + 210;
    one[13] = two + 230;
    one[14] = two + 238;
    one[15] = two + 245;
    one[16] = two + 256;
}

__attribute__((noinline)) int
kernel(uint64_t** one, const int one_size, uint64_t* two, const int two_size)
{

    // simple kernel
    int sum = 0;
    for (auto i = 0; i < one_size-1; ++i) {
        printf("[app]: processing %d\n", i);

        for (auto j = one[i]; j < one[i+1]; ++j) {
            if (*j % 2) { ++sum; }
        }
    }

    return sum;
}

int
main(int argc, char** argv)
{
    const int one_size = 17; 
    const int two_size = 256;
    const int num_nodes_pf = 2;
    const int num_triggers_pf = 1;
    const int num_edges_pf = 1;

    uint64_t** one = (uint64_t**) malloc(sizeof(uint64_t*) * one_size);
    uint64_t* two = (uint64_t*) malloc(sizeof(uint64_t) * two_size);

    (void) initialize(one, two, one_size, two_size);

    pf_params_t params(num_nodes_pf, num_edges_pf, num_triggers_pf, 4);
    pf_enable_t enable;

    printf("one: %p\none bound: %p\n", one, one + one_size);
    printf("two: %p\ntwo bound: %p\n", two, two + two_size);
    printf("&params: %p\n", &params);
    printf("&enable: %p\n", &enable);
    
    // register traversal pattern
    params.RegisterNode(one, one_size, 0);
    params.RegisterNode(two, two_size, 1);
    params.RegisterTravEdge(0, 1, PointerBounds_uint64_t);
    params.RegisterTrigEdge(0, 0, UpToOffset, NeverSquash);

    // now, call into sim
    SimUser(PF_SET_PARAM, (long unsigned int) &params);
    SimUser(PF_SET_ENABLE, (long unsigned int) &enable);

    // =================================================
    // =================================================

    SimRoiStart();
    printf("gapbs: bfs_enable_t @ %p\n", &enable); // don't ask why: absolutely need this print here
                                                 // to pass correct address
    SimUser(PF_ENABLE, (long unsigned int) &enable);

    if (SimInSimulator() and !enable.is_enabled()) {
        enable.wait();
    }

    int result = 0;
    printf("result before: %d\n", result); 
    result = kernel(one, one_size, two, two_size);
    printf("result after: %d\n", result);

    SimRoiEnd();
    SimUser(PF_DISABLE,0);

    // =================================================
    // =================================================

    free(one);
    free(two);
    return 0;
}
