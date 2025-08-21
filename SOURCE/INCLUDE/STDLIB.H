#define STDLIB_DEBUG_LEVEL 0

#define NULL (void*)0

typedef (*fp) ();		// returns address of a Class-memberfunction

// standard new function for objects
inline void* __builtin_new(unsigned int size)
{
   void *p;

   p = (void*)valloc(size);
   return p;
}

// standard delete function for objects
inline void __builtin_delete(void *ptr)
{
	if (vfree(ptr) == -1)
	{

#if STDLIB_DEBUG_LEVEL == 1
		printk("stdlib.h: unable to delete object at address: %00000008x",ptr);
#endif
	}

}

	

