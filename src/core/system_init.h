/**
 * @file system_init.h
 * @brief System initialization interface (source directory version)
 */

#ifndef SRC_SYSTEM_INIT_H
#define SRC_SYSTEM_INIT_H

#include "hal_interface.h"

#ifdef __cplusplus
extern "C"
{
#endif

    hal_status_t system_init(void);
    hal_status_t system_deinit(void);
    bool is_system_initialized(void);
    void print_init_progress(void);

#ifdef __cplusplus
}
#endif

#endif // SRC_SYSTEM_INIT_H
