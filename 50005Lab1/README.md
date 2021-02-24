# 50005Lab1
I was able to finish this lab with the help of Darryl Tan and Toh Kai Feng, who were nice enough to patiently explain to me how to go about doing it and showed me segments of their code for me to understand, after which I tried to implement it from my memory with understanding. I am grateful for their help and would like to take the time here to show my gratitude to the two of them.

For TODO4, Darryl taught me how to do it in a very clear and concise manner.

My method was as follows: 

task status checks whether the current job in the buffer is completed or not

isalive checks if worker process is still alive


I first initialised a variable gettask that varies between 1 and 0. if the variable was set to 1, then we would need to get the task by moving into the while loop and looping through the processes. 

gettask is like a flag to help you get out of the while loop, bcos you want to keep looping and checking through all your processes and jobs to assign a new task to any process that is currently idle. so once you find a process that is idle, you can give it the current job, and then you can go read the next job from the file

So basically the job buffer array and children processes array kinda correspond according to index so job_buffer[0] is the job for the worker process at children_processes[0], so when you check that the job at index 0 not completed, you’ll move to index 1 to check the next job/process pair

change gettask to 0 not bcos you don’t need to get it anymore, but bcos you’ve alr assigned the current task to a process, so you need to read the next task from the file and assign it to the next process