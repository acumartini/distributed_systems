Adam Martini
CIS630 Distributed Systems
Project 1
4-20-14

Simulating Many Random Walk on a Large Graph:

Use the "make" command to compile rws.cpp and utils.cpp into an executable "rws".  Use the following
command and arguments to run the executable:

./rws <path/to/dataset> <number_of_steps>

Arguments:
	<path/to/dataset> - The network file to load and process.
	<number_of_steps> - The number of rounds to run during the simulation many random walk.

The program will load the network data and then perform the given number of steps of credit updates 
to simulate a many random walk.  There are data ouput options for each step that are commented out
(lines 169-177) for efficiency.  To create and store distribution metrics, uncomment these lines and
create a "data" directory local to your executable.
