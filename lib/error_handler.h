/**
 * @file error_handler.h
 * @author TheRedCowboy
 * @brief Header file for error handling in the SYN Flood attack script. 
 * This file defines error codes for various error conditions that may occur during the execution of the script,
 *  such as invalid arguments, wrong IP or port format, and aborted execution. 
 * These error codes can be used throughout the codebase to provide consistent error handling and reporting.
 * @version 1.0
 * @date 2026-03-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#define ERR_TOO_MANY_ARGS 100 // Error code for when the user provides more arguments than expected. The script expects a maximum of 3 arguments (destination IP and port), and this error indicates that the user has exceeded that limit.
#define ERR_WRONG_IP_FORMAT 120 // Error code for when the user provides an IP address in an incorrect format. The expected format is x.x.x.x, where each octet is a number between 0 and 255. This error indicates that the provided IP address does not conform to this format, which could lead to issues in socket creation and packet sending.
#define ERR_WRONG_PORT_FORMAT 121 // Error code for when the user provides a port number that is not within the valid range of 1 to 65535. This error indicates that the provided port number is either too low (below 1) or too high (above 65535), which could lead to issues in socket creation and packet sending.
#define ERR_ABORTED 400 // Error code for when the user chooses to abort the execution of the script. This can occur when the user is prompted to continue without providing any arguments and decides not to proceed with the default configuration. This error indicates that the execution was intentionally stopped by the user.

#endif