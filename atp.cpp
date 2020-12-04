#define ATP_IS_IMPLEMENTATION
#include "atp.h"
#include "atp_config.atp"

//Used for memory allocation in free, malloc, realloc
#ifdef _WIN32 
#include <corecrt_malloc.h>
#else
#include <stdlib.h>
#endif
#define ATP_REALLOC realloc
#define ATP_CALLOC calloc
#define ATP_FREE free

#ifdef _DEBUG
#define ATP_ASSERT(x) if(!(x)) __debugbreak();
#else
#define ATP_ASSERT(X)
#endif 


namespace ATP
{
    //Singleton that is initialized on static initialization
    TestType_Registry* testtype_registry;
    static int32 registry_capacity;

    TestType* add_multi_testtype_to_registry(const char* name_, uint32 buffer_size_)
    {
        if (testtype_registry == (TestType_Registry*)0)
        {
            testtype_registry = (TestType_Registry*)ATP_CALLOC(1, sizeof(TestType_Registry));      //creating the global_testtypes buffer. pointer is initially zero initialized.
        }
        if (testtype_registry->front == (TestType*)0) //If buffer is not initilized yet
        {
            testtype_registry->no_of_testtypes = 0;
            registry_capacity = ATP_TESTTYPE_BUFFER_INIT_CAPACITY;
            testtype_registry->front = (TestType*)ATP_CALLOC(registry_capacity, sizeof(TestType));
        }
        //making room for new testtype in registry
        testtype_registry->no_of_testtypes++;
        if (testtype_registry->no_of_testtypes > registry_capacity)
        {
            registry_capacity = registry_capacity + ATP_TESTTYPE_BUFFER_OVERFLOW_ADDON;
            TestType* temp = (TestType*)ATP_REALLOC(testtype_registry->front, registry_capacity * sizeof(TestType));
            ATP_ASSERT(temp);   //couldn't realloc
            testtype_registry->front = temp;
        }

        TestType new_testtype = {};
        new_testtype.type = TestTypeFormat::MULTI;
        new_testtype.name = name_;
        if (buffer_size_ != 0)  //default initilize with specified size
        {
            new_testtype.tests.finished_tests = 0;
            new_testtype.tests.size = buffer_size_;
            new_testtype.tests.front = (TestInfo*)ATP_CALLOC(buffer_size_, sizeof(TestInfo));
        }
        TestType* temp = testtype_registry->front;
        temp = temp + (testtype_registry->no_of_testtypes - 1);
        *temp = new_testtype;
        return temp;
    }


    //for SINGULAR testtypes
    TestType* add_singular_testtype_to_registry(const char* name_)
    {
        if (testtype_registry == (TestType_Registry*)0)
        {
            testtype_registry = (TestType_Registry*)ATP_CALLOC(1, sizeof(TestType_Registry));      //creating the global_testtypes buffer. pointer is initially zero initialized.
        }
        if (testtype_registry->front == (TestType*)0) //If buffer is not initilized yet
        {
            testtype_registry->no_of_testtypes = 0;
            registry_capacity = ATP_TESTTYPE_BUFFER_INIT_CAPACITY;
            testtype_registry->front = (TestType*)ATP_CALLOC(registry_capacity, sizeof(TestType));
        }
        //making room for new testtype in registry
        testtype_registry->no_of_testtypes++;
        if (testtype_registry->no_of_testtypes > registry_capacity)
        {
            registry_capacity = registry_capacity + ATP_TESTTYPE_BUFFER_OVERFLOW_ADDON;
            TestType* temp = (TestType*)ATP_REALLOC(testtype_registry->front, registry_capacity * sizeof(TestType));
            ATP_ASSERT(temp);   //couldn't realloc 
            testtype_registry->front = temp;
        }

        TestType new_testtype = {};
        new_testtype.type = TestTypeFormat::SINGULAR;
        new_testtype.info = { 0 };
        new_testtype.name = name_;

        TestType* temp = testtype_registry->front;
        temp = temp + (testtype_registry->no_of_testtypes - 1);
        *temp = new_testtype;
        return temp;
    }

    TestType* lookup_testtype(const char* name)
    {
        TestType *tt = testtype_registry->front;
        for (int i = 0; i < testtype_registry->no_of_testtypes; i++)
        {
            if (name == tt->name)
            {
                return tt;
            }
            tt++;
        }
        return 0;
    }

    f64 get_ms_from_test(TestType& test)
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        f64 time_elapsed = (test.info.test_run_cycles * 1000 / (f64)frequency.QuadPart);
        return time_elapsed;
    }
}
