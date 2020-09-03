#include <iostream>
#include "atp.h"

typedef int  int32;
typedef long long int64;
typedef unsigned long long uint64;
typedef double f64;


ATP_REGISTER(with_profiling);
ATP_REGISTER(without_profiling);
ATP_REGISTER(blanktest);

void emptytest()
{
	int64 bla = 0;
	for (int64 i = 0; i < 100000; i++)
	{
		bla++;
	}
}

void nonemptytest()
{
	int64 bla = 0;
	for (int64 i = 0; i < 100000; i++)
	{
		ATP_BLOCK(blanktest);
		bla++;
	}
}

void test()
{
	ATP_START(without_profiling);
	emptytest();
	ATP_END(without_profiling);

	ATP_START(with_profiling);
	nonemptytest();
	ATP_END(with_profiling);


}

void print_all_tests()
{
	ATP::TestType* it = ATP::testtype_list_global_front;
	if (it == nullptr)
	{
		return;
	}
	while (it->next_node != nullptr)
	{
		std::cout << "Test '" << it->name << "' ran for :" << it->info.test_run_cycles << " cycles. ( or " << ATP::get_ms_from_test(*it) << " milliseconds)\n";
		it = it->next_node;
	}
}

int main()
{
	test();
	print_all_tests();
}
