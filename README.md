# ATProfiler
 A single header library to profile code.
 
 ## Features: 
 - Low overhead (Doesn't interfere with profile times)
 - Simple to use interface (Single line to profile a block)
 - Multi-Test Support. Allows a dynamic array of tests to be generated and called upon during runtime.
 - Single header. Super easy to import into any project.
 - No initialization code require. Just plug in and use. (Allocates all required memory for tests before 'main')  


## Example:
### Using single-test types:

```
ATP_REGISTER(test_allocators);
ATP_REGISTER(marena_alloc);

static void test_allocators(PL &pl)
{
	ATP_BLOCK(test_allocators);

	int* new_p[10000];

	for (int i = 0; i < 10000; i++)
	{
		new_p[i] = new int();
		*new_p[i] = i;
	}

	ATP_START(marena_alloc);
	for (int i = 0; i < 10000; i++)
	{
		new_p[i] = (int*)MARENA_PUSH(&pl.memory.main_arena, sizeof(int), "array");
		*new_p[i] = i;
	}
	ATP_END(marena_alloc);
}
```


### Using multi-test types:

```
//Use 0 to specify number of tests will be assigned during runtime. Otherwise use the total number of tests (tests will be allocated by ATP) 
ATP_REGISTER_M(Tiles, 0);
static b32 render_tile_from_camera(uint32 tile_no, RayCastTools& tools)
{
	ATP_BLOCK_M(Tiles, tile_no);
 render_tile(tile_no, tools);
}
```
