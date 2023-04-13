#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

int main(int argc, char* argv[])
{
    picobench::runner r;
    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    //r.set_default_state_iterations({15024});
    r.set_default_state_iterations({128, 1024, 16384});
    r.parse_cmd_line(argc, argv);
    return r.run();
}

