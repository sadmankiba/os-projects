#include <linux/printk.h>

#include <stdarg.h>

void sl_core_log(void *ptr, const char *level, const char *name, const char *text, ...) {
    va_list args;
    va_start(args, text);
    vprintk(text, args);
    va_end(args);
}
