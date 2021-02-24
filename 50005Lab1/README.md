# 50005Lab1
I was able to finish this lab with the help of Darryl Tan and Toh Kai Feng, who were nice enough to patiently explain to me how to go about doing it and showed me segments of their code for me to understand, after which I tried to implement it from my memory with understanding. I am grateful for their help and would like to take the time here to show my gratitude to the two of them.

For TODO4, Darryl taught me how to do it in a very clear and concise manner.

My method was as follows: 

task_status checks whether the current job in the buffer is completed or not

isalive checks if worker process is still alive

the "job" object has attributes of duration, type and status. 

I first initialised a variable gettask that varies between 1 and 0. gettask is a flag to help me get out of the while loop, because I want to keep looping and checking through all the processes and jobs to assign a new task to any process that is currently idle. So once I have found a process that is idle, I can assign it the current job, and then I can go read the next job from the file.

The job buffer array and children processes array correspond according to index so job_buffer[0] is the job for the worker process at children_processes[0], so when I check that the job at index 0 not completed, I’ll move to index 1 to check the next job/process pair.

for the legal termination of worker processes that are still alive, if the processes are still alive after the first round of looping is finished, then I change the different job fields such that it terminates. 