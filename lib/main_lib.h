/**
 * @file main_lib.h
 * @author TheRedCowboy
 * @brief Header file for main_lib.c, which contains utility functions for the SYN Flood attack script,
 * including input validation for IP addresses and ports, as well as a function to update configuration values in an INI file.
 * The check_ip_octet function validates the format of an IP address.
 * The check_port function validates that a given port number is within the valid range.
 * The update_ini_key function allows for updating specific key-value pairs in a configuration INI file
 * within the script, enabling dynamic configuration changes without modifying the source code directly.
 * @version 1.0
 * @date 2026-03-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef MAIN_LIB_H
#define MAIN_LIB_H

_Bool check_ip_octet(char* addr);
_Bool check_port(int port);
void update_ini_key(const char *filename, const char *key, const char *new_value);

#endif 
