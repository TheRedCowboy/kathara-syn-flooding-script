/**
 * @file main_lib.c
 * @author TheRedCowboy
 * @brief This file contains utility functions for the SYN Flood attack script, 
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main_lib.h"

#define FIXED_LENGTH 3

/**
 * @brief Validates the format of an IP address by checking each octet for correct length and numeric value.
 * The function splits the input string into octets based on the '.' delimiter and checks that each octet is a valid number between 0 and 255.
 * The function returns 0 if the IP address is valid, and 1 if it is invalid. 
 * This validation is crucial for ensuring that the script can create sockets and send packets to valid IP addresses without encountering errors due to malformed input.
 * 
 * @param addr A string representing the IP address to be validated.
 * @return _Bool Returns 0 if the IP address is valid, or 1 if it is invalid.
 */
_Bool check_ip_octet(char* addr){

    /* The function initializes four character arrays (fr_ott, sn_ott, th_ott, fh_ott) to store the individual octets of the IP address */
    char fr_ott[FIXED_LENGTH+1]={0};
    char sn_ott[FIXED_LENGTH+1]={0};
    char th_ott[FIXED_LENGTH+1]={0};
    char fh_ott[FIXED_LENGTH+1]={0};

    int curr_ott_count=1; 
    int curr_cph_count=0;

    _Bool ret_val=0;

    while(*addr != '\0'){
        if(curr_cph_count > 3) ret_val=1; // Check if the current character position within the octet exceeds the fixed length of 3 characters. 
                                          // If it does, set the return value to 1, indicating an invalid IP address format.
        
        if(*addr!='.'){ // If the current character is not a dot, it is part of an octet. Increment the character position counter and check if the character is a digit. 
                      // If it is not a digit, set the return value to 1, indicating an invalid IP address format. 
                      // Depending on the current octet count, store the character in the corresponding octet array (fr_ott, sn_ott, th_ott, fh_ott).
            curr_cph_count++;

            if(curr_cph_count > FIXED_LENGTH)  ret_val=1;
            if(!isdigit(*addr)) ret_val=1;

            switch (curr_ott_count){ // Use a switch statement to determine which octet array to store the current character in based on the current octet count. 
                                    // The first octet is stored in fr_ott, the second in sn_ott, the third in th_ott, and the fourth in fh_ott. 
                                    // If the octet count exceeds 4, set the return value to 1, indicating an invalid IP address format.
                case 1:
                    fr_ott[curr_cph_count-1]=*addr;
                    break;
                case 2:
                    sn_ott[curr_cph_count-1]=*addr;
                    break;
                case 3: 
                    th_ott[curr_cph_count-1]=*addr;
                    break;
                case 4: 
                    fh_ott[curr_cph_count-1]=*addr;
                    break;
                
                default:
                    ret_val=1;
                    break;
            }
        }else{
            curr_cph_count=0;
            curr_ott_count++;
        }

        addr++;
    }

    if(curr_ott_count < 4) ret_val=1;

    /* Check if any octet exceeds 255 */
    if(atoi(fr_ott) > 255) ret_val=1;
    if(atoi(sn_ott) > 255) ret_val=1;
    if(atoi(th_ott) > 255) ret_val=1;
    if(atoi(fh_ott) > 255) ret_val=1; 

    return ret_val;
}

/**
 * @brief Validates that a given port number is within the valid range of 1 to 65535.
 * The function checks if the provided port number is greater than or equal to 1 and less than or equal to 65535.
 * 
 * @param port An integer representing the port number to be validated.
 * @return _Bool Returns 0 if the port number is valid (within the range of 1 to 65535), or 1 if it is invalid (outside this range). 
 * This validation is important to ensure that the script can create sockets and send packets to valid ports without encountering errors due to invalid port numbers.
 */
_Bool check_port(int port){
    if(port >=1 && port <= 65535) return 0;
    else return 1;
}

/**
 * @brief Updates a specific key-value pair in an INI configuration file. 
 * The function reads the existing INI file, searches for the specified key, 
 * and updates its value with the new value provided. 
 * If the key is not found, it can be added to the file.
 *  The function handles file operations and ensures that the INI file is correctly updated with the new configuration values. 
 * This allows for dynamic configuration changes without modifying the source code directly, 
 * enabling users to easily update settings such as destination IP and port for the SYN Flood attack script.
 * 
 * @param filename The path to the INI file that needs to be updated. This file should exist and be accessible for reading and writing.
 * @param key The specific key in the INI file that needs to be updated. 
 * The function will search for this key in the file and update its corresponding value.
 * @param new_value The new value that will be assigned to the specified key in the INI file. This value will replace the existing value associated with the key, or be added if the key does not already exist in the file.
 */
void update_ini_key(const char *filename, const char *key, const char *new_value) {
    FILE *file = fopen(filename, "r");
    FILE *temp = fopen("../config/temp.ini", "w"); // Create a temporary file to write the updated content. This approach allows for safely updating the INI file without risking data loss in case of errors during the update process.
    
    if (!file || !temp) {
        if (file) fclose(file);
        if (temp) fclose(temp);
        return;
    }

    char line[256];
    char search_key[100];
    sprintf(search_key, "%s=", key); // Construct the search key by appending an equals sign to the provided key. This allows for accurate searching of the key in the INI file, ensuring that we match the key followed by an equals sign, which is the standard format for key-value pairs in INI files.

    while (fgets(line, sizeof(line), file)) { // Read each line from the original INI file. The fgets function is used to read lines of text, and the loop continues until the end of the file is reached.
        if (strncmp(line, search_key, strlen(search_key)) == 0) {
            fprintf(temp, "%s=%s\n", key, new_value); // If the current line starts with the search key, write the updated key-value pair to the temporary file. This effectively updates the value for the specified key in the INI file.
        } 
        else {
            fputs(line, temp);
        }
    }

    fclose(file);
    fclose(temp);

    remove(filename);    
    rename("../config/temp.ini", filename);  // Replace the original INI file with the updated temporary file. This step finalizes the update process by ensuring that the changes are saved to the original INI file, allowing the script to read the new configuration values in subsequent operations.
}

