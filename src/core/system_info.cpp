/**
 * @file system_info.cpp
 * @brief System information display implementation
 */

#include "system_info.h"
#include <stdio.h>

void display_system_info(void) {
    printf("\n=== System Information ===\n");
    printf("Multi-Channel Diagnostic Test Rig - Pico W\n");
    printf("Version: 1.0.0\n");
    printf("Build Date: %s %s\n", __DATE__, __TIME__);
    printf("==========================\n\n");
}
