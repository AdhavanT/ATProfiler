#ifndef ATP_HEADER_IS_INCLUDED
#define ATP_HEADER_IS_INCLUDED

/*
-------------------------------<ATProfiler>--------------------------------------------------------
                 For more info -> github.com/AdhavanT/ATProfiler

add the "atp_config.atp" file to configure macros

ATP is a simple-to-use profiler built for games and other applications.
ATP focuses on low performance overhead and being easy to use.
The trade-off is building the various "TestType" buffers at program start-up.
ATP builds a global buffer to hold all "TestType"s. 
ATP does not automatically clean up this global testtype registery or "MULTI" type testtypes. 
If concerned with cleanup, do manually by getting registry with get_testtype_registry().

Usage Example:

        REGISTER_TEST(timer_name);    //This must be in global scope

        void do_stuff()
        {
            ATP_START(timer_name);
            ..do stuff i guess...
            ATP_END(timer_name);
        }

 Example using scoped object :

        REGISTER_TEST(timer_name);    //This must be in global scope

        void do_stuff()
        {
            ATP_BLOCK(timer_name);   //Creates a simple object whose destructor calls ATP_END() at scope end.
            ..do stuff i guess...
        }
TODO for anyone: This Library doesn't use many C++ features (mostly just syntax) and can be easily be ported to C.
-------------------------------</ATProfiler>-------------------------------------------------------
*/
#include "atp_config.atp"

//Dependencies 




namespace ATP
{
    enum TestTypeFormat
    {
        SINGULAR, MULTI
    };

    typedef int b32;
    typedef int  int32;
    typedef unsigned int uint32;
    typedef long long int64;
    typedef unsigned long long uint64;
    typedef float f32;
    typedef double f64;

    struct TestInfo
    {
        uint64 test_run_cycles;
    };

    struct MultiTest_Buffer    //meant for MULTI testtypes. A buffer of TestInfos. 
    {
        uint32 size;    //the amount of testinfos the buffer can hold
        uint32 finished_tests;  
        TestInfo* front = 0;
    };

    struct TestType
    {
        TestTypeFormat type;
        const char* name;
        union       //A discrimated union used for both SINGULAR and MULTI testtypes
        {
            //SingularTestInfo
            TestInfo info;

            //MultiTestInfo (a buffer of TestInfos)
            MultiTest_Buffer tests;

        };
    };

    //singleton buffer that holds all TestTypes 
    struct TestType_Registry
    {
        int32 no_of_testtypes;
        TestType* front = (TestType*)0;
    };
}
namespace ATP
{
    //pointer to TestType_Registry singleton
    extern TestType_Registry* testtype_registry;

    //looks up a testtype from name. 
    TestType* lookup_testtype(const char* name);

    //NOTE: this is ad-hoc. don't use for high performance.
    f64 get_ms_from_test(TestType& test);

    TestType* add_singular_testtype_to_registry(const char* name_);

    TestType* add_multi_testtype_to_registry(const char* name_, uint32 buffer_size_);
}


//Defining macros :

// ATP_GET_TESTTYPE(name)          //quick macro to alias pointer to specified TestType in same compilation unit
// ATP_EXTERN_REGISTER(name)       //Use to Call a TestType registered in a different compilation unit

//For SINGULAR testtypes
// ATP_REGISTER(name)              //Use to Register a TestType. Place in global scope.
// ATP_EXTERN_REGISTER(name)       //Use to Call a TestType registered in a different compilation unit
// ATP_START(name)                 //Use to start test 
// ATP_END(name)                   //Use to end test and register test info
// ATP_BLOCK(name)                 //A scoped test. Automatically calls ATP_START and calles ATP_END at end of scope.

//For MULTI testtypes 
// ATP_REGISTER_M(name, default_size)            //Use to Register a TestType. Place in global scope. 
// ATP_START_M(name)                             //Use to start test (same as ATP_START())
// ATP_END_M(name, index)                        //Use to end timer and register testinfo of index test
// ATP_BLOCK_M(name, index)                      //Scoped ATP_START() and ATP_END() block


//-----------------------<Windows>-------------------------------------
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN     
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN


//Used to call to a test registered in a different compilation unit 
#define ATP_EXTERN_REGISTER(name) \
     extern ATP::TestType *ATPTESTTYPE_##name;

//quick macro to alias pointer to specified TestType in same compilation unit
#define ATP_GET_TESTTYPE(name) (ATPTESTTYPE_##name)

//---------------------<SINGULAR TESTTYPE>-----------------------------
//Use to Register a SINGULAR TestType. Place in global scope.
#define ATP_REGISTER(name) \
     ATP::TestType *ATPTESTTYPE_##name= ATP::add_singular_testtype_to_registry(#name)


#ifdef ATP_USE_QPC
//Use to start test (using qpc to time)
#define ATP_START(name) \
        LARGE_INTEGER ATP_START_CYCLES_##name;\
        QueryPerformanceCounter(&ATP_START_CYCLES_##name)

//Use to end test
#define ATP_END(name) \
        LARGE_INTEGER ATP_END_CYCLES_##name;\
        QueryPerformanceCounter(&ATP_END_CYCLES_##name);\
        ATPTESTTYPE_##name->info.test_run_cycles = (ATP_END_CYCLES_##name.QuadPart - ATP_START_CYCLES_##name.QuadPart)\

namespace ATP
{
    struct ATP_SCOPED_TEST_BLOCK
    {
        TestType* testtype;
        LARGE_INTEGER cycles;
        ATP_SCOPED_TEST_BLOCK(TestType* tt)
            :testtype(tt)
        {
            QueryPerformanceCounter(&cycles);
        }
        ~ATP_SCOPED_TEST_BLOCK()
        {
            LARGE_INTEGER tmp;
            QueryPerformanceCounter(&tmp);
            testtype->info.test_run_cycles = tmp.QuadPart - cycles.QuadPart;
        }
    };
}

#else   //!ATP_USE_QPC
//Use to start test (using rdtsc to time)
#define ATP_START(name) \
        uint64 ATP_START_CYCLES_##name = __rdtsc()

//Use to end test
#define ATP_END(name) \
        ATP_END_CYCLES_##name = __rdtsc() - ATP_START_CYCLES_##name;\
        ATPTESTTYPE_##name->info.test_run_cycles = ATP_END_CYCLES_##name


namespace ATP
{
    struct ATP_SCOPED_TEST_BLOCK
    {
        TestType* testtype;
        uint64 cycles;
        ATP_SCOPED_TEST_BLOCK(TestType* tt)
            :testtype(tt)
        {
            cycles = __rdtsc();
        }
        ~ATP_SCOPED_TEST_BLOCK()
        {
            testtype->info.test_run_cycles = __rdtsc() - cycles;
            testtype->hits++;
        }
    };
}

#endif
//A scoped test. Automatically calls ATP_START and calles ATP_END at end of scope.
#define ATP_BLOCK(name) \
        ATP::ATP_SCOPED_TEST_BLOCK ATP_SCOPED_TEST_##name(ATPTESTTYPE_##name)
//---------------------</SINGULAR TESTTYPE>----------------------------

//---------------------<MULTI TESTTYPE>--------------------------------
//For registering MULTI testtype. Set default_size to 0 to avoid automatic initialization.
#define ATP_REGISTER_M(name, default_size) \
     ATP::TestType *ATPTESTTYPE_##name = ATP::add_multi_testtype_to_registry(#name, default_size)

//Use to start test. Same as ATP_START()
#define ATP_START_M(name) ATP_START(name)

#ifdef ATP_USE_QPC
//Use to end test. Include with index of MULTI testtype to add new testinfo to.
#define ATP_END_M(name, index) \
        LARGE_INTEGER ATP_END_CYCLES_##name;\
        QueryPerformanceCounter(&ATP_END_CYCLES_##name);\
        ATPTESTTYPE_##name->tests.front[(index)].test_run_cycles = (ATP_END_CYCLES_##name.QuadPart - ATP_START_CYCLES_##name.QuadPart);\
        ATPTESTTYPE_##name->tests.finished_tests++

namespace ATP
{
    struct ATP_SCOPED_MULTITEST_BLOCK
    {
        TestType* testtype;
        LARGE_INTEGER start_cycles;
        uint32 index;
        ATP_SCOPED_MULTITEST_BLOCK(TestType* tt, uint32 index_)
            :testtype(tt), index(index_)
        {
            QueryPerformanceCounter(&start_cycles);
        }
        ~ATP_SCOPED_MULTITEST_BLOCK()
        {
            LARGE_INTEGER end;
            QueryPerformanceCounter(&end);
            testtype->tests.front[index].test_run_cycles = end.QuadPart - start_cycles.QuadPart;
            testtype->tests.finished_tests++;
        }
    };
}

#else
//Use to end test. Include with index of MULTI testtype to add new testinfo to.
#define ATP_END_M(name, index) \
        ATP_END_CYCLES_##name = __rdtsc() - ATP_START_CYCLES_##name;\
        ATPTESTTYPE_##name->tests.front[(index)].test_run_cycles = ATP_END_CYCLES_##name;\
        ATPTESTTYPE_##name->tests.finished_tests++

namespace ATP
{
    struct ATP_SCOPED_MULTITEST_BLOCK
    {
        TestType* testtype;
        uint64 start_cycles;
        uint32 index;
        ATP_SCOPED_MULTITEST_BLOCK(TestType* tt, uint32 index_)
            :testtype(tt), index(index_)
        {
            start_cycles = __rdtsc();
        }
        ~ATP_SCOPED_MULTITEST_BLOCK()
        {
            uint64 end;
            end = __rdtsc() - start_cycles;
            testtype->tests.front[index].test_run_cycles = end;
            testtype->tests.finished_tests++;
        }
    };
}
#endif
//A scoped MULTI test. Automatically calls ATP_START and calles ATP_END at end of scope.
#define ATP_BLOCK_M(name, index) \
        ATP::ATP_SCOPED_MULTITEST_BLOCK ATP_SCOPED_TEST_##name(ATPTESTTYPE_##name, (index))
//---------------------</MULTI TESTTYPE>-------------------------------

#endif // !Win32
//-----------------------</Windows>-------------------------------------


//Removing atp_config macros 
#define ATP_CONFIG_UNDEF
#include "atp_config.atp"
#undef ATP_CONFIG_UNDEF

#endif // !ATP_HEADER_IS_INCLUDED
