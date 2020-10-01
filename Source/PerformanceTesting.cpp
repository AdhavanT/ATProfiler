#include <iostream>
#include "atp.h"

typedef int  int32;
typedef long long int64;
typedef unsigned long long uint64;
typedef double f64;


ATP_REGISTER(with_profiling);
ATP_REGISTER(without_profiling);
ATP_REGISTER(blanktest);

void emptytest(int32 iterations)
{
	int64 bla = 0;
	for (int64 i = 0; i < iterations; i++)
	{
		bla++;
	}
}

void nonemptytest(int32 iterations)
{
	int64 bla = 0;
	for (int64 i = 0; i < iterations; i++)
	{
		ATP_BLOCK(blanktest);
		bla++;
	}
}

void test(int32 iterations)
{
	ATP_START(without_profiling);
	emptytest(iterations);
	ATP_END(without_profiling);

	ATP_START(with_profiling);
	nonemptytest(iterations);
	ATP_END(with_profiling);


}

void print_all_tests()
{
	for(int i = 0 ; i < ATP::global_testtypes->length; i++)
	{
		ATP::TestType it = ATP::global_testtypes->at(i);
		std::cout << "Test '" << it.name << "' ran for :" << it.info.test_run_cycles << " cycles. ( or " << ATP::get_ms_from_test(it) << " milliseconds)\n";
	}
}

int main()
{
	test(1000);
	print_all_tests();
}
