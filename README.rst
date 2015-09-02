======================
DISCLAIMER
==========
STH Hypervisor is a research project and is continuously being changed and developed and may contain stability issues and security vulnerabilities at this stage. 


It is offered to you WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE.TXT for more information.


What is STH?
============

The SICS Thin Hypervisor (STH) software is a small portable hypervisor designed for the ARM architecture, with the main purpose to improve the security of embedded systems through the isolation properties that virtualization can provide. This is achieved by having a very thin software layer, the hypervisor, solely running in the processors privileged mode, managing and maintaining the resource allocation and access policies to the guests running on top of it. This allows the hypervisor to limit the power of the guest by para-virtualizing it to run in unprivileged mode, effectively creating isolation between the different components. With the help of the hypervisor, one can run a commodity OS kernel such as Linux and its user appli- cations together with trusted security services in different execution environments isolated from each other.

The STH software is currently primarily used for research purposes and is dual licensed GPL/Commercial. See the accompanying license file for more details. 


HARDWARE
========

The STH hypervisor currently runs on the 32-bit ARM v5 and v7 architecture. A number of popular development boards are supported,
although new targets can be added with ease.


USING IT
========

See the "doc" folder for detailed information.
