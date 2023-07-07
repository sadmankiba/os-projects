#include <linux/printk.h>
#include <linux/kernel.h> /* snprintf, va_list */

void mod_log(void *ptr, const char *level, const char *name, const char *text, ...) {
    char msg[125];
    va_list args;

    snprintf(msg, 125, "%-10s: %s", name, text);
    va_start(args, text);
    vprintk(msg, args);
    va_end(args);
}
