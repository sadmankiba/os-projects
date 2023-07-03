#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>

MODULE_DESCRIPTION("Simple module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

struct Foo {
	int a;
	int b;
};

struct Bar {
    int c;
    double d;
};

struct Comb {
    union {
        struct Foo foo;
        struct Bar bar;
    };  
};

static int union_kmem_init(void)
{
	struct kmem_cache *cache;
	struct Comb *comb;

	pr_debug("union_kmem init!\n");
	
	cache = KMEM_CACHE(Comb, 0);
	comb = kmem_cache_alloc(cache, GFP_KERNEL); 
	(comb->foo).a = 1;
	(comb->foo).b = 2;
	pr_debug("(comb->foo).a = %d, (comb->foo).b = %d\n", (comb->foo).a, (comb->foo).b);
    (comb->bar).c = 5;
    (comb->bar).d = 6.89;
    pr_debug("(comb->bar).c = %d, (comb->bar).d = %f\n", (comb->bar).c, (comb->bar).d);
    
	kmem_cache_free(cache, comb);
	kmem_cache_destroy(cache);
	return 0;
}

static void union_kmem_exit(void)
{
	pr_debug("union_kmem exit!\n");
}

module_init(union_kmem_init);
module_exit(union_kmem_exit);
