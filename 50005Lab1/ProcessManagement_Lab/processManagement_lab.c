#include "processManagement_lab.h"

/**
 * The task function to simulate "work" for each worker process
 * TODO#3: Modify the function to be multiprocess-safe 
 * */
void task(long duration)
{
    // simulate computation for x number of seconds
    usleep(duration*TIME_MULTIPLIER);

    // TODO: protect the access of shared variable below
    // update global variables to simulate statistics
    while (true) {
        int waitResult = sem_wait(sem_global_data);
        if (waitResult ==0) {
            ShmPTR_global_data->sum_work += duration;
            ShmPTR_global_data->total_tasks ++;
            if (duration % 2 == 1) {
                ShmPTR_global_data->odd++;
            }
            if (duration < ShmPTR_global_data->min) {
                ShmPTR_global_data->min = duration;
            }
            if (duration > ShmPTR_global_data->max) {
                ShmPTR_global_data->max = duration;
            }
            usleep(5*TIME_MULTIPLIER);
            sem_post(sem_global_data);
            return;
        }
        if (waitResult ==-1) {
            exit(EXIT_FAILURE);
        }
    }
    
}


/**
 * The function that is executed by each worker process to execute any available job given by the main process
 * */
void job_dispatch(int i){

    // TODO#3:  a. Always check the corresponding shmPTR_jobs_buffer[i] for new  jobs from the main process
    //          b. Use semaphore so that you don't busy wait
    //          c. If there's new job, execute the job accordingly: either by calling task(), usleep, exit(3) or kill(getpid(), SIGKILL)
    //          d. Loop back to check for new job 
    while (true){
        if (shmPTR_jobs_buffer[i].task_status == 1) {
            int waitResult = sem_wait(sem_jobs_buffer[i]);
            if (waitResult ==0) {
                if (shmPTR_jobs_buffer[i].task_type == 't') {
                    task(shmPTR_jobs_buffer[i].task_duration);
                    shmPTR_jobs_buffer[i].task_status = 0; // no job or job cleared
            }
            else if (shmPTR_jobs_buffer[i].task_type == 'w') {
                usleep(shmPTR_jobs_buffer[i].task_duration * TIME_MULTIPLIER);
                shmPTR_jobs_buffer[i].task_status = 0; // no job or job cleared
            }
            else if (shmPTR_jobs_buffer[i].task_type == 'z') {
                shmPTR_jobs_buffer[i].task_status = 0; // no job or job cleared
                exit(3);
            }
            else if (shmPTR_jobs_buffer[i].task_type == 'i') {
                shmPTR_jobs_buffer[i].task_status = 0; // no job or job cleared
                kill(getpid(), SIGKILL);
            }
        }
            if (waitResult==-1){
                exit(EXIT_FAILURE);
            }
        }
    }
}

/** 
 * Setup function to create shared mems and semaphores
 * **/
void setup(){

    // TODO#1:  a. Create shared memory for global_data struct (see processManagement_lab.h)
    //          b. When shared memory is successfully created, set the initial values of "max" and "min" of the global_data struct in the shared memory accordingly
    // To bring you up to speed, (a) and (b) are given to you already. Please study how it works. 

    //          c. Create semaphore of value 1 which purpose is to protect this global_data struct in shared memory 
    //          d. Create shared memory for number_of_processes job struct (see processManagement_lab.h)
    //          e. When shared memory is successfully created, setup the content of the structs (see handout)
    //          f. Create number_of_processes semaphores of value 0 each to protect each job struct in the shared memory. Store the returned pointer by sem_open in sem_jobs_buffer[i]
    //          g. Return to main


// part a: which helps us create a shared memory for the global_data structure - this has been done for us, thanks Prof Nat
    ShmID_global_data = shmget(IPC_PRIVATE, sizeof(global_data), IPC_CREAT | 0666);
    if (ShmID_global_data == -1){
        printf("Global data shared memory creation failed\n");
        exit(EXIT_FAILURE);
    }
    ShmPTR_global_data = (global_data *) shmat(ShmID_global_data, NULL, 0);
    if ((int) ShmPTR_global_data == -1){
        printf("Attachment of global data shared memory failed \n");
        exit(EXIT_FAILURE);
    }

// part b: to set the initial values of the maximum and minimum for the global_data structure in the shared memory - this has been done for us, thanks Prof Nat
    ShmPTR_global_data->max = -1;
    ShmPTR_global_data->min = INT_MAX;
// part c: creating a semaphore with value 1 to protect the global_data structure, i would first open a semaphore
    // from the document, it says that sem_open must be used and gives the various input arguments.
    //sem_t *sem_open(const char *name, int oflag, ...);      
    char *sem_global_name="semglobaldata";
    sem_global_data = sem_open("sem_global_name", O_CREAT | O_EXCL, 0644, 1);
    // i then check if there is an existing semaphore with the same name;
    while(true){
        if (sem_global_data == SEM_FAILED){
            sem_unlink("sem_global_name");
            sem_global_data = sem_open("sem_global_name", O_CREAT | O_EXCL, 0644, 1);
    }
    else {
        break;
    }
}


//part d and e: setting up of the shared memory for jobs and feeling in the content accordingly
    ShmID_jobs = shmget(IPC_PRIVATE,sizeof(job)*number_of_processes, IPC_CREAT | 0666);
    // checking for errors in the shared memory initialized
    if (ShmID_jobs ==-1){
        exit(EXIT_FAILURE);
    }
    // if there is no error the segment shall be attached
    shmPTR_jobs_buffer = (job *) shmat(ShmID_jobs, NULL, 0);
    //checking if there is an error again
    if ((int)shmPTR_jobs_buffer==-1){
        exit(EXIT_FAILURE);
    }
//part f:
    for (int i =0; i < number_of_processes; i++){
        shmPTR_jobs_buffer[i].task_status = 0;
        char *semjob_id = malloc(sizeof(char)*64);
        sprintf(semjob_id,"semjobs%d",i);

        sem_jobs_buffer[i]=sem_open(semjob_id,O_CREAT | O_EXCL,0644,0);
        // same as above in part c to check if there was a semaphore with the same name and the steps needed to be taken to unlink and reinitialise 
        while(true){
            if (sem_jobs_buffer[i]==SEM_FAILED){
                sem_unlink(semjob_id);
                sem_jobs_buffer[i]=sem_open(semjob_id,O_CREAT | O_EXCL,0644,0);
            }
            else
                break;
                }
                free(semjob_id);
    }
//part g returning back to main! 
    return;
}
/**
 * Function to spawn all required children processes
 **/
void createchildren(){
    // TODO#2:  a. Create number_of_processes children processes

    pid_t pid[number_of_processes];

    for (int i =0; i<number_of_processes; i++){
        pid[i] = fork();
        if (pid[i]<0){
            fprintf(stderr,"FAILED TO CREATE FORK YO");
            exit(EXIT_FAILURE);
        }
        else if (pid[i]==0){
            //part b: if the childPID is valid then we shall store the PID of children i respectively
            job_dispatch(i);
            break;
            }
        }
    //          b. Store the pid_t of children i at children_processes[i]
    //          c. For child process, invoke the method job_dispatch(i)
    //          d. For the parent process, continue creating the next children
    //          e. After number_of_processes children are created, return to main 
    return;
}
void create_final_job(int i, int type, int duration, int status) {
    sem_wait(sem_global_data);
    shmPTR_jobs_buffer[i].task_type = type;
    shmPTR_jobs_buffer[i].task_duration = duration;
    shmPTR_jobs_buffer[i].task_status = status;
    sem_post(sem_global_data);
}
/**
 * The function where the main process loops and busy wait to dispatch job in available slots
 * */
void main_loop(char* fileName){

    // load jobs and add them to the shared memory
    FILE* opened_file = fopen(fileName, "r");
    char action; //stores whether its a 'p' or 'w'
    long num; //stores the argument of the job 
    int get_task; //this is an integer that we will alternate between 1 and 0, whereby if its 1, then we would need to get the task and 0 otherwise! 

    while (fscanf(opened_file, "%c %ld\n", &action, &num) == 2) { //while the file still has input

        //TODO#4: create job, busy wait
       get_task = 1; // since gettask is set to 1, we definitely need to get the task. as such, we move into the while loop.

        while (get_task == 1) {
            // create job, busy wait
            // part a. Busy wait and examine each shmPTR_jobs_buffer[i] for jobs that are completed and empty by checking that shmPTR_jobs_buffer[i].task_status == 0(this is given!).
            // also need to ensure that the process i IS alive using waitpid(children_processes[i], NULL, WNOHANG). This WNOHANG option will not cause main process to block when the child is still alive. waitpid will return 0 if the child is still alive, 1 otherwise!
            for (int i =0; i < number_of_processes; i++) {

                int isalive = waitpid(children_processes[i], NULL, WNOHANG);
                
                // part b. If both conditions in (a) is satisfied update the contents of shmPTR_jobs_buffer[i], and increase the semaphore using sem_post(sem_jobs_buffer[i])
                if (shmPTR_jobs_buffer[i].task_status == 0 && isalive == 0) {
                    shmPTR_jobs_buffer[i].task_duration = num;
                    shmPTR_jobs_buffer[i].task_type = action;
                    shmPTR_jobs_buffer[i].task_status = 1;
                    get_task = 0; // no need to get task anymore.
                    // part d. Then, update the shmPTR_jobs_buffer[i] for this process. Afterwards, don't forget to do sem_post as well 
                    sem_post(sem_jobs_buffer[i]);
                    // c. Break of busy wait loop, advance to the next task on file 
                    break;
                }
                else if (isalive != 0) {
                    // d. Otherwise if process i is prematurely terminated, revive it. You are free to design any mechanism you want. The easiest way is to always spawn a new process using fork(), direct the children to job_dispatch(i) function.
                    // i will use the same method as create_children() in #todo 2! 
                    pid_t pid = fork();
                    children_processes[i]=pid;
                    if (pid < 0) {
                        exit(EXIT_FAILURE);
                    } 
                    else if (pid == 0) {
                        job_dispatch(i);
                        return;
                    }
                    else{
                        shmPTR_jobs_buffer[i].task_type = action;
                        shmPTR_jobs_buffer[i].task_duration = num;
                        shmPTR_jobs_buffer[i].task_status =1;
                        sem_post(sem_jobs_buffer[i]);
                        get_task =0;
                        break;
                    }
                }
            }       
        }      
    // e. The outermost while loop will keep doing this until there's no more content in the input file. 
    }
    fclose(opened_file);

    //printf("Main process is going to send termination signals\n");

    // TODO#4a: Design a way to send termination jobs to ALL worker that are currently alive 
    // loop through all the processes that are currently present
    int counter=0;
    while(counter!=number_of_processes){
    for (int i=0; i < number_of_processes; i++) {
            if (shmPTR_jobs_buffer[i].task_type=='z') {
                continue; //busywait for children to get their job
            }
            if (shmPTR_jobs_buffer[i].task_status==-1) {
                counter++;
                create_final_job(i,'z',0, 0);
                continue;
            }
            if (shmPTR_jobs_buffer[i].task_status==0) {
                create_final_job(i,'z',0, 1);
                counter++;
                sem_post(sem_jobs_buffer[i]);
        }
    }
}
    //wait for all children processes to properly execute the 'z' termination jobs
    int process_waited_final = 0;
    pid_t wpid;
    while ((wpid = wait(NULL)) > 0){
        process_waited_final ++;
    }
    // print final results
    printf("Final results: sum -- %ld, odd -- %ld, min -- %ld, max -- %ld, total task -- %ld\n", ShmPTR_global_data->sum_work, ShmPTR_global_data->odd, ShmPTR_global_data->min, ShmPTR_global_data->max, ShmPTR_global_data->total_tasks);
}   

void cleanup(){
    //TODO#4: 
    // 1. Detach both shared memory (global_data and jobs)
    // 2. Delete both shared memory (global_data and jobs)
    // 3. Unlink all semaphores in sem_jobs_buffer

    // i happened across this within the testmain_todo3.txt ! Thank you Prof Nat for making my life easier, truly appreciate it.
    //detach and remove shared memory locations
    int detach_status = shmdt((void *) ShmPTR_global_data); //detach
    if (detach_status == -1) printf("Detach shared memory global_data ERROR\n");
    int remove_status = shmctl(ShmID_global_data, IPC_RMID, NULL); //delete
    if (remove_status == -1) printf("Remove shared memory global_data ERROR\n");
    detach_status = shmdt((void *) shmPTR_jobs_buffer); //detach
    if (detach_status == -1) printf("Detach shared memory jobs ERROR\n");
    remove_status = shmctl(ShmID_jobs, IPC_RMID, NULL); //delete
    if (remove_status == -1) printf("Remove shared memory jobs ERROR\n");

    //unlink all semaphores before exiting process
    int sem_close_status = sem_unlink("semglobaldata");
    //no need print if closed properly, if not will get marks deducted! 
    for (int i = 0; i<number_of_processes; i++){
        char *sem_name = malloc(sizeof(char)*16);
        sprintf(sem_name, "semjobs%d", i);
        sem_close_status = sem_unlink(sem_name);
        //no need print if closed properly, if not will get marks deducted!
        free(sem_name);
    }
}


// Real main
int main(int argc, char* argv[]){

    //printf("Lab 1 Starts...\n");

    struct timeval start, end;
    long secs_used,micros_used;

    //start timer
    gettimeofday(&start, NULL);

    //Check and parse command line options to be in the right format
    if (argc < 2) {
        printf("Usage: sum <infile> <numprocs>\n");
        exit(EXIT_FAILURE);
    }


    //Limit number_of_processes into 10. 
    //If there's no third argument, set the default number_of_processes into 1.  
    if (argc < 3){
        number_of_processes = 1;
    }
    else{
        if (atoi(argv[2]) < MAX_PROCESS) number_of_processes = atoi(argv[2]);
        else number_of_processes = MAX_PROCESS;
    }

    setup();
    createchildren();
    main_loop(argv[1]);

    //parent cleanup
    cleanup();

    //stop timer
    gettimeofday(&end, NULL);

    double start_usec = (double) start.tv_sec * 1000000 + (double) start.tv_usec;
    double end_usec =  (double) end.tv_sec * 1000000 + (double) end.tv_usec;

    printf("Your computation has used: %lf secs \n", (end_usec - start_usec)/(double)1000000);


    return (EXIT_SUCCESS);
}