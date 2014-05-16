CIS630, Spring 2014, Term Project (Part I), Due date: April 24, 2014

Please complete the following information, save this file and submit it along with your program

Your First and Last name: Adam Martini
Your Student ID: 951282210

Does your program compile on ix: Yes

How should we compile your program on ix (please provide short but specific commands):
Use the "make" command.

Does your program run on ix: Yes

Does your program have a limit for the number of nodes in the input graph? Yes
If yes, what is the limit on graph size that your program can handle?
I am using a unsigned long data type to define the size limits of my graph.  This limits the number of nodes
to 2^32 = 4294967295 nodes.

How long does it take for your program to read the input file on ix?
Roughly 41 seconds.

How long does it take for your program (on average) to complete each round of processing on ix?
Roughly 24 seconds with parallel updates using OpenMP.

IMPLEMENTATION NOTES:
The program will load the network data and then perform the given number of rounds of credit updates 
to simulate a many random walk.  There are data output options for each step that are commented out
(lines 178-186) for efficiency.  To create and store distribution metrics, uncomment these lines and
create a "data" directory local to your executable.

CONVERGENCE REPORT:
Please see "martini_CIS630_proj1.pdf" for a report on the convergence of the algorithm, which includes
average distribution distances between time steps and standard deviations over time.

