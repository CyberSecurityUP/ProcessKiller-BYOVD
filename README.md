# ProcessKiller-BYOVD

## Overview
This project demonstrates the concept of BYOVD, "Bring Your Own Vulnerable Driver," a technique utilized by researchers and security practitioners to study the exploitation of driver vulnerabilities and the manipulation of kernel-mode privileges on Windows systems. The focus is on the utilization of a specific IOCTL (Input/Output Control) code, `0x82730030`, in conjunction with the `viragt64` driver, which is known for its exploitable characteristics.

## About viragt64 Driver
The `viragt64` driver, listed under the ID [7edb5602-239f-460a-89d6-363ff1059765](https://www.loldrivers.io/drivers/7edb5602-239f-460a-89d6-363ff1059765), is a well-documented example of a driver that exposes several operational risks due to improper handling of IOCTL requests. These vulnerabilities allow for various operations, such as reading and writing to arbitrary memory locations, thereby providing a pathway to elevate privileges or compromise system integrity.

## Project Structure
The core component of this project is a C++ application that interfaces with the `viragt64` driver using the WinAPI. The application is responsible for:

- Loading the driver using standard service control manager APIs.
- Sending custom commands to the driver via the IOCTL interface.
- Handling responses and errors from the driver to mimic realistic interaction scenarios.

## Security Disclaimer
This project is intended for educational and research purposes only. The demonstration of the BYOVD technique and interactions with the `viragt64` driver should be conducted in a controlled environment, such as a virtual machine not connected to live networks. Users are advised to exercise caution and understand the implications of interacting with kernel-mode drivers.

## Usage
To run the project, compile the provided C++ code and execute the binary with administrative privileges on a system where the `viragt64` driver is installed and loaded. Ensure that the environment is secure and isolated from production networks to prevent unintended security risks.
