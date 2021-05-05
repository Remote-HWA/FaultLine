# FaultLine: Software-based Fault Injection on Memory Transfers

FaultLine is a software-based fault injection attack vector. It uses delay-lines (located in memory controllers) as memory transfer glitch injectors.

## Content

This repository provides: 
- The **source code** required to reproduce the FaultLine Baremetal and Linux attacks.
- A tutorial to reproduce the baremetal experiments.

## Baremetal Requirements
- A Zynq-7000 dev board.
- Xilinx Vivado + Vitis Software Suite

## Baremetal tutorial 
1. **Launch** Vivado and create a [platform project](https://www.xilinx.com/html_docs/xilinx2020_2/vitis_doc/ogx1607514458074.html) for your board (xsa file).
2. **Launch** Vitis and create a platform project (name: *FaultLineHP*) using the xsa file generated.
3. **Double click** on the platform.spr file and modify the domains to obtain one domain per CPU core.

<p align="center">
<img src="https://user-images.githubusercontent.com/83603699/117171378-1a87b600-adcb-11eb-86cf-0cc55294fb1f.PNG" width="600" height="200">
</p>
<p align="center"> Figure 1: One domain per CPU core (CPU0 and CPU1) <p align="center">
  
4. **Create** an empty application project for the adversary (name: *adversary_cpu0*) and select processor *ps7_cortexa9_0*
5. **Create** an empty application project for the victim (name: *victim_cpu1*) and select processor *ps7_cortexa9_1* 
6. **Add** the content provided [here](https://github.com/LAbbbs/FaultLine/tree/main/Baremetal_FIA/adversary_CPU0) in the *adversary_cpu0* project.
7. **Add** the content provided [here](https://github.com/LAbbbs/FaultLine/tree/main/Baremetal_FIA/victim_CPU1) in the *victim_cpu1* project.

<p align="center">
<img src="https://user-images.githubusercontent.com/83603699/117189693-c5559f80-adde-11eb-92f3-2dfca130672d.PNG" width="280" height="340">
</p>
<p align="center"> Figure 2: Project Arborescence <p align="center">
  
9. For each project, add its include paths to the Directories in *Properties->Settings->Directories*
10. **Compile** the victim and adversary projects.
11. **Go to** Project->Run Configurations, Right click on Single Application Debug and select New configuration.
12. **Go to** the Application tab and check ps7_cortexa9_1 so both projects will be launched simultenaously in different cores.
13. **Select** *Apply*.
14. **Power up** the board, open a serial terminal and Run the configuration. The welcome prompt should appear (if you successfully setup the project you should see a message from each core. CPU#0 and CPU#1) 

<p align="center">
<img src="https://user-images.githubusercontent.com/83603699/117175009-a6e7a800-adce-11eb-97b8-f68f9f9311b3.PNG" width="500" height="320">
</p>
<p align="center"> Figure 3: Welcome Prompt <p align="center">
 
14. **Enter** ```calib``` to find the faulty delay-line values
15. **Enter** ```piret```, ```PFA``` or ```rtest``` to reproduce the attacks presented in the paper.

<p align="center">
<img src="https://user-images.githubusercontent.com/83603699/117186432-1b284880-addb-11eb-8487-ba70a9bf8b82.gif" width="800" height="450">
</p>
<p align="center"> Figure 4: Calibration and Piret Attack <p align="center">

## Linux Requirements
- A Zynq-7000 dev board.
- A **micro SD** card.
- Linux distribution (*linaro-jessie-developer-20161117-32*).

## Linux Tutorial 
To do 



