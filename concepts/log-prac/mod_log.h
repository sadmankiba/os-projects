#ifndef _CORE_LOG_H
#define _CORE_LOG_H

#define MOD_PART1_NAME "Car"
#define MOD_PART2_NAME "Helicopter"

void mod_log(void *ptr, const char *level, const char *name, const char *text, ...);

#define mod_log_dbg(_ptr, _name, _text, ...) \
	mod_log((_ptr), KERN_DEBUG, (_name), (_text), ##__VA_ARGS__)

#define mod_log_err(_ptr, _name, _text, ...) \
    mod_log((_ptr), KERN_ERR, (_name), (_text), ##__VA_ARGS__)

#endif