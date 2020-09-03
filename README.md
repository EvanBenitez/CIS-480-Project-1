## Project 480

I going to use this for a general state of the project.

I think I'm going to place all the things that need to be worked on in the issues tab. I have some ideas on how to create some of the output that we need.

I moved everything for the mipsParser into one file. I think it will be less confusing, if I just leave the test main in the same .c file. My plan is to do this from now on, and once everything is finished, we can just move all the relevant code to one file.

I'm currently working on the assumption that the code for every instruction will be a separate function. If we have each one return an int, that will be the equivalent of one MIPS instruction.
