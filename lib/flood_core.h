/**
 * @file flood_core.h
 * @author TheRedCowboy
 * @brief Header file for flood_core.c, which contains the main logic for executing a SYN Flood attack by crafting and sending raw TCP packets. 
 * This header defines the function prototype for execute_denial(), which is responsible for loading the raw packet, 
 * manipulating it (including IP spoofing and checksum recalculation), and sending it in an infinite loop to the target. 
 * The function also handles error cases and provides feedback on the execution status of the attack.
 * @version 1.0
 * @date 2026-03-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef FLOOD_CORE_H
#define FLOOD_CORE_H

_Bool execute_denial();

#endif