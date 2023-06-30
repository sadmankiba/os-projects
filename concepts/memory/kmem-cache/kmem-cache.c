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
static int kcache_init(void)
{
	struct kmem_cache *cache;
	struct Foo *foo;

	pr_debug("kcache init!\n");
	
	cache = kmem_cache_create("my_cache", 100, 0, 0, NULL);
	foo = kmem_cache_alloc(cache, GFP_KERNEL);
	foo->a = 1;
	foo->b = 2;
	pr_debug("foo->a = %d, foo->b = %d\n", foo->a, foo->b);
	kmem_cache_free(cache, foo);
	kmem_cache_destroy(cache);
	return 0;
}

static void kcache_exit(void)
{
	pr_debug("kcache exit!\n");
}

module_init(kcache_init);
module_exit(kcache_exit);
