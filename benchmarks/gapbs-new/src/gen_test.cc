// user defined headers
#include <pf_interface.h>
#include <sim_api.h>

// STL headers
#include <cassert>
#include <iostream>

// class that is the size of a cacheline
class POD
{
public:
    POD() : data()
    {
        for (int i = 0; i < 64; ++i) { data[i] = (char) i; }
    }

    char data[64];

    int num_even() const
    {
        int count = 0;

        for (int i = 0; i < 64; ++i) {
            if (data[i] % 2 == 0) {
                ++count;
            }
        }
        return count;
    }
};

// helper function for initialization
void
initialize(int* one, POD* two, const int one_size, const int two_size)
{
    assert(one_size == 16);
    assert(two_size > 63);

    // this doesn't really matter cuz we don't traverse farther than this
    for (int i = 0; i < two_size; ++i) {
        two[i] = POD();
    }

    // hardcode in sequence to make sure debugging is easier
    one[0]  = 0;
    one[1]  = 9;
    one[2]  = 4;
    one[3]  = 31;
    one[4]  = 23;
    one[5]  = 63;
    one[6]  = 55;
    one[7]  = 44;
    one[8]  = 21;
    one[9]  = 16;
    one[10] = 15;
    one[11] = 60;
    one[12] = 42;
    one[13] = 40;
    one[14] = 1;
    one[15] = 2;
}

__attribute__((noinline)) int
kernel(int* one, const int one_size, POD* two, const int two_size)
{

    // simple kernel
    int sum = 0;
    for (auto i = 0; i < one_size; ++i) {
        printf("[app]: processing %d\n", i);
        printf("one access: %p\n", one + i);
        int tmp = *(one+i);
        sum += two[tmp].num_even();
    }

    return sum;
}

int
main(int argc, char** argv)
{
    const int one_size = 16;
    const int two_size = 64;
    const int num_nodes_pf = 2;
    const int num_triggers_pf = 1;
    const int num_edges_pf = 1;

    int* one = (int*) malloc(sizeof(int) * one_size);
    POD* two = (POD*) malloc(sizeof(POD) * two_size);

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
    params.RegisterTravEdge(0, 1, BaseOffset_int32_t);
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
