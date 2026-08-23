# Context 

Instead of writing one-off scripts, I have created this repo with the intention of running different experiments using QuEST and tracking results across a range of configurations and distrubuted 
hardware set-ups (mainly hosted on distributed VM). 

I also aim to research the main drivers of memory communication overhead in distrubuted sytems (within the context of state vector simulation), and consider ways to implement some memory communication 
overhead reduction techniques (relates to my Hons project).  

The overall goal is to learn and have fun with QuEST, alongside laying some foundational work for my 4th year Hons project. 


## VM Architecture

The following diagram describes the VM setup I have been using on Azure. It comprises of a head node that controls and runs processes on a cluster of compute nodes (CPU VM instances at the moment but pending
quota increase I may be able to get access to GPU VM types). 

The CPU types I have been using so far are from the FX SKU and have 8 cores with 128GB RAM each. The largest statevector simulation I was able to successfully run with this set up was 35 qubits, where the 
statevector was initialised to have a full random state and a H gate was applied to each qubit index. 
*

# Goals

1. Write functions to simulate some well known QCs. 
2. Run QCs under different configurations and conditions, i.e.:
	- Number of qubits being simulated (dictates size of state vector).
	- Number of ranks for each node (more ranks per nodes results in higher communication costs). 
	- Circuit complexity (a range of circuits in differing gate complexity and size).
3. Maintain track of each run/experiment and compare across the rest. Features to compare can be:
	- Circuit runtime (not the best to show pure communication time delta but still nice to see).
	- Number of different gate operations and their properties:
		- Local gate operations update amplitudes in place.
		- Local memory gate operations update amplitudes by computing a linear combination of amplitudes in the same node.
		- Global gate operation require amplitudes to be communnicated across nodes, which incurrs an MPI communication time overheard.
   among some...
4. Think of some ways to try and reduce this communication overhead by implementing some techniques like cache-blocking or other docuemented methods (such as utilising PGAS) in reducing communication 
overhead in distributed systems 
5. Evaluate the difference between using these implemented techniques and not. 
6. Report results and findings (if any) that may be useful for Hons project 

# Status

**Current: Setting up repo**
*Next Steps: Init dev env* 


# Credits

This repo has used the following materials in its development: 

**QuEST - Quantum Exact Simulation Toolkit**
- qtechtheory (https://qtechtheory.org/)
*(https://github.com/QuEST-Kit/QuEST
https://quest.qtechtheory.org/about/*


**Energy Efficieny of Quantum Statevector Simulation at Scale**
- Jakub Adamski
- James Peter Richings
- Oliver Thomson Brown
*(https://arxiv.org/pdf/2308.07402)*

