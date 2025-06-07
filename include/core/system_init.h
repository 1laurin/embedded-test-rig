#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H
#include "platforms/common/hal_interface.h"
hal_status_t system_init(void);
bool is_system_initialized(void);
#endif
