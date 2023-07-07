#ifndef _CORE_LOG_H
#define _CORE_LOG_H

#define SL_CORE_LINK_AN_LOG_NAME "link-an"

void sl_core_log(void *ptr, const char *level, const char *name, const char *text, ...);

#define sl_core_log_dbg(_ptr, _name, _text, ...) \
	sl_core_log((_ptr), KERN_DEBUG, (_name), (_text), ##__VA_ARGS__)

#define sl_core_log_err(_ptr, _name, _text, ...) \
    sl_core_log((_ptr), KERN_ERR, (_name), (_text), ##__VA_ARGS__)

#endif