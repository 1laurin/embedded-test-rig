/**
 * @file system_init.h
 * @brief System initialization interface for multi-channel diagnostic test rig
 *
 * This file provides the interface for initializing all HAL subsystems
 * and hardware components in the correct order.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

#include "../../platforms/common/hal_interface.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize HAL and all hardware subsystems
     * @return HAL_OK on success, error code otherwise
     */
    hal_status_t system_init(void);

    /**
     * @brief Deinitialize all systems and cleanup resources
     * @return HAL_OK on success, error code otherwise
     */
    hal_status_t system_deinit(void);

    /**
     * @brief Get system initialization status
     * @return true if system is fully initialized, false otherwise
     */
    bool is_system_initialized(void);

    /**
     * @brief Print initialization progress and results
     */
    void print_init_progress(void);

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_INIT_H