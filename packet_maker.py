"""
Packet Master Script
------------------------
This script creates custom raw TCP packets using Scapy and saves them 
in binary format based on configurations provided in an .ini file.

Author: TheRedCowboy
"""


import logging
import os
import configparser
from scapy.all import *
from scapy.all import IP
from scapy.all import TCP


logging.getLogger("scapy.runtime").setLevel(logging.ERROR) #Deactivate Warnings

"""
    Save the raw packet in a binary format

    :param raw: The raw packet
    :type raw: scapy.layers.l3raw.Packet
"""
def save_raw(raw):
    
    os.makedirs("../generated",exist_ok=True) #Create dir if doesen't exists

    with open("../generated/packet.bin","wb") as fopen: 
        fopen.write(bytes(raw)) #Write the package in binary

"""
    Create the raw TCP packet and call the save_raw function
    to save the packet in a binary format

    :param dest_port: The port of the victim
    :type dest_port: int
    :param dest_ip: The IP Address of the victim
    :type dest_ip: string
    
""" 
def create_raw(dest_port,dest_ip):
    ip_header = IP(dst = dest_ip,src=RandIP()) #Create the ip Header with the destination IP
    tcp = TCP(sport = RandShort(), dport = dest_port, flags = "S")

    raw_pkt = ip_header / tcp 
    
    save_raw(raw_pkt)
  
"""
    Main Function
"""
def main():
    config = configparser.ConfigParser() #Create Parser obj
    config.read('../config/config.ini') #Read the configuration file

    create_raw(config.getint('infos', 'dest_port'),config.get('infos','dest_ip')) #Call the function to create the raw packet passing the arguments 
                                                                                  #taken from the configuration file


if __name__ == "__main__":
    # Entry point of the script
    main() 