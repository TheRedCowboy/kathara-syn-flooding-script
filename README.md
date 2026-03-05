# kathara-syn-flooding-script
![C](https://img.shields.io/badge/Language-C-blue.svg)
![Python](https://img.shields.io/badge/Language-Python-3776AB?logo=python&logoColor=white)
![Kathara](https://img.shields.io/badge/Environment-Kathara-orange.svg)
![Status](https://img.shields.io/badge/Status-Educational_Research-green.svg)

<p align="center">
   <img width="225" height="107" alt="synfldpng" src="https://github.com/user-attachments/assets/20c22a28-d503-4398-b308-d351835f5108" />
</p>

A hybrid C/Python tool for high-performance TCP SYN Flooding attacks within Kathara network environments.

## Preface
This project was developed for a university challenge to explore a hybrid approach, leveraging the simplicity of Python (Scapy) for TCP SYN packet crafting and the low-level efficiency of C for high-speed raw socket injection. As an educational prototype focused on the speed of the transmission loop, this software is not intended to be a fully optimized production tool.

## Overview
This project demonstrates a multi-stage approach to network stress testing:
* **Generation**: A Python script uses Scapy to craft a raw TCP/IP packet template.
* **Execution**: A C core engine handles heap allocation, raw sockets, and the Denial of Service (DoS) attack.
* **Environment**: Specifically optimized for Kathara labs to analyze network resilience.

## Key Features
* **Hybrid Core**: Python for packet design + C for high-speed transmission.
* **Low-Level Control**: Manual IP spoofing and raw checksum recalculation ($RFC 793$).
* **Memory Efficiency**: Dynamic heap buffering for raw packet ingestion.
* **Lab Ready**: Includes a pre-configured Kathara scenario (PC1, PC2, PC3, R1).

## Project Structure
| File/Dir | Description |
| :--- | :--- |
| `main.c` | Entry point of the application; manages the attack flow and user input. |
| `main_lib.c`| Helper library for command parsing, configuration, and input validation. |
| `flood_core.c` | Main C engine (Socket AF_INET, Raw Packet Ingestion). |
| `packet_maker.py` | Python script for initial Scapy-based packet assembly. |
| `examples/` | Kathara lab configuration files for testing. |
| `Makefile` | Build system for the C executable. |

---

## Briefing

### Simulation Environment
This project is developed and tested using **Kathara**, a network virtualization tool based on Docker. This environment is used for security and ethical reasons, ensuring that high-speed traffic remains isolated within a sandboxed local network, preventing any impact on real-world infrastructure.

### Modern Mitigation and Attack Efficacy
In a modern production environment, a basic SYN Flood attack is largely ineffective. Contemporary Operating Systems and network appliances utilize several layers of defense, primarily SYN Cookies. This mechanism allows the server to avoid allocating resources (*TCP Backlog*) until the three-way handshake is fully completed, effectively neutralizing the flood.

### Vulnerability Setup (Target Node Configuration)
To demonstrate the theoretical impact of the attack and observe the exhaustion of the TCP backlog, security measures must be manually disabled on the target. The following commands must be included in the pc3.startup file (*the victim server*) to bypass modern kernel protections:

```bash
# pc3.startup (Target Server)

su - # Execute as root
sysctl -w net.ipv4.tcp_syncookies=0         # Disables SYN Cookies protection
sysctl -w net.ipv4.tcp_abort_on_overflow=0   # Forces the kernel to ignore overflows instead of resetting
```

> [!Note]
> If you're using kathara your lab must be executed with **Super User (root)** or **Administrator privileges** to modify the kernel's network parameters effectively.

## Before Start : Prerequisites

Before running the simulation, ensure your environment is properly set up. You will need a Linux-based system (like the Kathara laboratory nodes) with the following tools installed:

- **GCC (GNU Compiler Collection)**: Required to compile the C source code for the attack tool. It is usually pre-installed in most Linux distributions.

- **Python 3**: Necessary to run any auxiliary scripts or management tools.

- **Scapy**: A powerful Python library used for packet manipulation and sniffing.

To install the missing components on a Debian-based system, you can use:

```bash
sudo apt-get update && sudo apt-get install gcc python3 python3-scapy
```

## Quick Start

### 1. Use the Project 

To use this project within a Kathara environment, you must place the compiled binary and the necessary scripts into the shared folder of your lab. This ensures the files are accessible from all nodes (*PC1, PC2, etc.*).

***Example Path (Windows)***:
`C:\Users\YourUser\Desktop\YourKatharaLab\shared\`

### 2. Build the project
Run the `make` command to compile the source files. The build system will automatically link main.c, main_lib.c, and flood_core.c, generating the final executable.

```bash
make
```

### 3. Launch Kathara Lab
After moving the binary to the `shared/` folder, start the network simulation. This command reads the configuration files and creates the virtual containers for all nodes (*PC1, PC2, PC3, and R1*).

```bash
kathara lstart
```

## Performing the Attack

### Phase 1: Target Service Setup & Monitoring
First, initialize the service on PC3. Once the server is running, you should start the monitoring tools to observe the real-time performance drop during the attack.

#### 1. Start the Target HTTP Server
Run the following command on PC3:
```bash
python3 -m http.server 80 &
```
> [!TIP]
> Run in Background: Using the ampersand (&) allows you to keep the terminal free.

<br>

 To stop the server later, use:

```bash
fuser -k 80/tcp
```

#### 2. Monitoring Tools
**Monitor CPU/RAM**:

```bash
top
```

**Monitor incoming SYN packets**:

```bash
tcpdump -i any "tcp[tcpflags] & tcp-syn != 0"
```

**Monitor TCP Backlog (half-open connections)**:

```bash
watch ss -n state syn-recv
```

> [!NOTE]
> ***The watch command will refresh the output every 2 seconds.*** You will see an increasing list of connections in the SYN-RECV state. Since the source IPs are spoofed and the target (PC3) is not receiving any final ACK, these entries will persist until they hit the timeout, eventually saturating the server's resources.

<br>
<br>

### Phase 2: Router Traffic Inspection
Monitor the transit of packets on the router (R1). This step is crucial to verify that the spoofed IP addresses generated by the attacker are correctly routed through the network.

```bash
tcpdump -i any "tcp[tcpflags] & tcp-syn != 0"
```
<br>
<br>

### Phase 3: Launch the Attack
Execute the binary from the attacker node (PC1). The C engine will start sending raw SYN packets at the maximum possible rate, bypasssing the standard TCP stack to optimize performance.

```bash
./syn_flood DEST_IP DEST_PORT
```

<br>
<br>

### Phase 4: Verification of Service Denial

From a legitimate user node (PC2), attempt to establish a connection to the server on PC3. The `curl` command will try to complete the TCP three-way handshake, but since the server's SYN-backlog is already saturated by the attack, the connection will likely remain in SYN_SENT state until it times out.

```bash
curl --connect-timeout 5 http://DEST_IP
```

> [!NOTE]
> **Observation**: Before starting the attack in Phase 3, running the curl command from PC2 will result in an immediate 200 OK response from the server. Once the attack is active, you will observe that requests from PC2 are no longer satisfied, demonstrating a successful Denial of Service (DoS) condition.

<br>
<br>

### Phase 5: Verification of Success
Go back to PC3 and check if the kernel is dropping connections due to queue saturation.

- If the counter increases, the SYN flood is successfully overwhelming the server's listen queue.

> [!WARNING]
> **Resource Exhaustion Warning**: Running the attack script for an extended period within a containerized environment may cause stability issues. In some cases, the intensive packet injection can cause the container engine to hang or impact the host operating system's performance (*this behavior was specifically observed during testing on Windows environments*). It is recommended to run the attack in short bursts to avoid system instability.

## Hybrid Core Logic: Packet Generation & Injection

- **Packet Fabrication and Binary Export**: The attack starts with packet_maker.py. Instead of sending packets directly, the Python script uses Scapy to craft a precise TCP SYN template. During execution, the project automatically creates a directory named `generated/`

   ```bash
    # The Python script saves the raw packet here:
    ./generated/packet.bin
   ```
- **C Engine Ingestion & Low-Level Manipulation**: The C core engine (*flood_core*) reads this binary file from the `generated/` folder. Once the raw data is loaded into the heap, the C code performs high-speed operations that Python cannot handle efficiently:
  - *Dynamic IP Spoofing*: It modifies the Source IP field in the IP header for every packet.
  - *Checksum Recalculation*: It recomputes the TCP and IP checksums ($RFC 793$) on the fly to ensure the packets are not dropped by the victim's stack.
  - *Raw Socket Injection*: It pushes the modified buffer directly into the network interface at maximum throughput.

## DISCLAIMER

> [!CAUTION]
> **Legal and Ethical Warning**: Although, as mentioned in the introduction, this type of attack is no longer effective against modern systems due to advanced kernel protections like SYN Cookies, this tool and the information provided are for **educational and ethical hacking purposes only**. 
>
> The sole purpose of this project is to demonstrate the theoretical mechanics of a SYN Flood attack within a **controlled, isolated laboratory environment** (*such as Kathara*).
>
> **Unauthorized use of these techniques against systems or networks without explicit permission is strictly illegal and unethical**. The author of this tool is not responsible for any misuse, damage, or legal consequences resulting from the application of this information in real-world scenarios.
