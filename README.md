## COSC 594 lab 1 jgraph
## Dominic Kennedy

1. What the program does
    This program draws frames of a metaball video using the bezier curve funcionality in jgraph.
    Together with the makefile a fully animated .mp4 video is generated
2. How to compile and run
    The code can be compiled by simply typing `make` this will produce the binary `metaballs`
    The program is run by typing `make run`.
    The makefile expects the `jgraph` binary to be present in the same directory.
    The makefile will take about a minute or so to fully run.
    Once finished there will be an `output.mp4` file in the same directory
    The program is randomly seeded so in order to generate 5 examples you can just run the makefile 5 times
3. Output example
   <video src='./example.mp4'> 
