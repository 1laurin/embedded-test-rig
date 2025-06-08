/**
 * @file system_info.h
 * @brief System information display interface
 */

#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

void display_system_info(void);
void print_startup_banner(void);
void print_init_progress(void);

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_INFO_H
