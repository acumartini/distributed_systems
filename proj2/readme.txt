CIS630, Spring 2014, Term Project (Part II), Due date: June 3, 2014

Please complete the following information, save this file and submit it along with your program

Your First and Last name: Adam Martini
Your Student ID: 951282210

What programming language did you use to write your code? C++

Does your program compile on ix-trusty: Yes

How should we compile your program on ix-trusty (please provide short but specific commands) use 'make'

Does your program run on ix-trusty: Yes

Does your program generate the correct results with 2 and 4 partitions on ix-trusty: Yes

Does your program have a limit for the number of nodes in the input graph? Yes
If yes, what is the limit on graph size that your program can handle?
I am using unsigned long for node id's, which limits the graph to 2^64 = 1.84467441e19 Nodes.  
However, MPI_Alltoallv requires integer arrays as input for buffer chunking, which imposes
an additional constraint on the number of Nodes partitions can exchange in a single communication.
Communication could be divided to allow for larger graphs, but the current limit imposed by the
integer constraint is 2^31-1 = 2,147,483,647 Nodes per partition.

How long does it take for your program to read the input file on ix-trusty?
Roughly 26 seconds on each partition.

How long does it take for your program (on average) to complete each round of processing on ix-trusty?
~1.5 seconds on each partition and ~3 seconds total for a round including communication overhead