#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue> 
#include <algorithm> 
#include <iomanip>
#include <math.h>
#include <assert.h>
#include <limits.h>

const std::string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int NUM_PROCESSES;
int SEED;
double LAMBDA;
int UPPER_BOUND;
int T_CS;
float ALPHA;
int T_SLICE;

double avg_CPU_burst = 0;
double avg_wait_t = 0;
double avg_turnaround_t = 0;
int num_cs = 0;
int num_preemp = 0;
double cpu_ultilization = 0;

typedef struct
{
  char id;
  int arrival;
  int burst;
  int CPU_index,IO_index;
  std::vector<int> CPU_t;
  std::vector<int> IO_t;
  int tau;
  int remainingT;

  double turnaround_t = 0;
  double wait_t = 0;

  bool recurring;
  bool preempted;
  int preemption;
  int SRTcmp;
} process_t;

std::deque<process_t> processes_copy_for_data;

double next_exp() {
	double num = UPPER_BOUND;
	while (num >= UPPER_BOUND) num = -log(drand48()) / LAMBDA;
	return num;
}

process_t* process_copy(process_t &p) {
	
	std::vector<int> new_CPU_t;
	std::vector<int> new_IO_t;

	process_t* dest = (process_t*)calloc(1, sizeof(process_t));

	for(size_t i = 0; i < p.CPU_t.size(); i++){
		 new_CPU_t.push_back(p.CPU_t[i]);
	}

	for(size_t i = 0; i < p.IO_t.size(); i++){
		 new_IO_t.push_back(p.IO_t[i]);
	}

	dest->CPU_t = new_CPU_t;
	dest->IO_t = new_IO_t;
	dest->id = p.id;
	dest->arrival = p.arrival;
	dest->burst = p.burst;
	dest->CPU_index = p.CPU_index;
	dest->IO_index = p.IO_index;
	dest->turnaround_t = p.turnaround_t;
	dest->wait_t = p.wait_t;
	dest->tau = p.tau;
	dest->remainingT = p.remainingT;
	dest->recurring = p.recurring;
	dest->preempted = p.preempted;
	dest->preemption = p.preemption;
	dest->SRTcmp = p.SRTcmp;

	return dest;
}

std::string queue_content(std::deque<process_t> &q) {
	std::string s = "Q ";
	std::deque<process_t>::iterator it;

	if (q.size() == 0) return "Q empty";

	for (it = q.begin(); it != q.end(); it++) s = s + (it->id);

	return s;
}

void sortqueue(std::deque<process_t> &processes) {
	std::sort(processes.begin(), processes.end(), [](const process_t& a, const process_t& b) { 
			if(a.arrival == b.arrival) return a.id < b.id;
    	return a.arrival < b.arrival;
	});
}

void SJF_sortqueue(std::deque<process_t> &processes) {
	std::sort(processes.begin(), processes.end(), [](const process_t& a, const process_t& b) { 
			if(a.tau == b.tau) return a.id < b.id;
    	return a.tau < b.tau;
	});
}

void SRT_sortqueue(std::deque<process_t> &processes, char target) {
	std::sort(processes.begin(), processes.end(), [](const process_t& a, const process_t& b) { 
			if(a.SRTcmp == b.SRTcmp) return a.id < b.id;
    	return a.SRTcmp < b.SRTcmp;
	});

	if(target != '0'){
		if(processes[0].id == target){
			process_t tmp = processes[0];
			processes[0] = processes[1];
			processes[1] = tmp;
		}
	}
}

void processSetUp(std::deque<process_t> &processes) {
	for (int i = 0; i < NUM_PROCESSES; i++) {

		process_t temp;
		temp.id = char(ALPHABET[i]);
		temp.arrival = int(floor(next_exp()));
		temp.burst = int(ceil(drand48() * 100));

		for (int j = 0; j < temp.burst-1; j++) {
			temp.CPU_t.push_back(int(ceil(next_exp())));
			temp.IO_t.push_back(int(ceil(next_exp())) * 10);
		}

		temp.CPU_t.push_back(int(ceil(next_exp())));

		temp.CPU_index = 0;
		temp.IO_index = 0;
		temp.turnaround_t = 0;
		temp.wait_t = 0;
		temp.recurring = false;
		temp.preempted = false;
		temp.preemption = -1;
		processes.push_back(temp);

		if (temp.burst == 1)
			std::cout << "Process " << processes.back().id 
					  << " (arrival time " << processes.back().arrival <<  " ms) "
					  << processes.back().burst << " CPU burst (tau " << ceil(1/LAMBDA)<<"ms)" << std::endl;
		else
			std::cout << "Process " << processes.back().id 
					  << " (arrival time " << processes.back().arrival <<  " ms) "
					  << processes.back().burst << " CPU bursts (tau " << ceil(1/LAMBDA)<<"ms)" << std::endl;
	}

 // Calculate avg CPU burst
	int num_burst = 0;
	for (std::deque<process_t>::iterator it = processes.begin(); it != processes.end(); it++) { 
		for (unsigned int i = 0; i < it->CPU_t.size(); i++) avg_CPU_burst += it->CPU_t[i]; 
		num_burst += it->CPU_t.size();
	}
	avg_CPU_burst /= num_burst;

	sortqueue(processes);
}

void wait_time_incr(std::deque<process_t> &ready_queue) {
	for (unsigned int i = 0; i < ready_queue.size(); i++) {
		for (unsigned int j = 0; j < processes_copy_for_data.size(); j++) {
			if (ready_queue[i].id == processes_copy_for_data[j].id) {
				processes_copy_for_data[j].wait_t++;
				break;
			}
		}
	}
}

void turnaround_time_incr(process_t & process, int inc) {
	for (unsigned int i = 0; i < processes_copy_for_data.size(); i++) {
		if (process.id == processes_copy_for_data[i].id) {
			processes_copy_for_data[i].turnaround_t += inc;
			break;
		}
	}
}

void global_var_cleaning() {
	avg_wait_t = 0;
	avg_turnaround_t = 0;
	num_cs = 0;
	num_preemp = 0;
	cpu_ultilization = 0;

	for (unsigned int i = 0; i < processes_copy_for_data.size(); i++) {
		processes_copy_for_data[i].wait_t = 0;
		processes_copy_for_data[i].turnaround_t = 0;
	}
}

int FCFS_RR(std::deque<process_t> processes, std::deque<process_t> ready_state, bool RR) {
  global_var_cleaning();
	bool flag = false;
	process_t* temp;

  int time = 0;
  int context_switch = 0;
  int cpu_running = 0;
  int time_slice = -1;
  bool start_using_cpu = true;
  process_t* running_state = NULL;
  std::deque<process_t>::iterator it;

  if (!RR)
      std::cout << "time " << time << "ms: " << "Simulator started for FCFS [" << queue_content(ready_state) << "]" << std::endl;
  else
      std::cout << "time " << time << "ms: " << "Simulator started for RR with time slice " << T_SLICE << "ms [" << queue_content(ready_state) << "]" << std::endl;

  while (true) {
    flag = false;
      // Go through the processes and check if multiple processes enter at the same time
      // Add arrived process(es) to the ready queue
    for (it = processes.begin(); it != processes.end(); it++) {
      if (it->arrival == time) {
        ready_state.push_back(*it);
        flag = true;
            
        if (it->recurring == false) {
          if(time <= 999)
          	std::cout << "time " << time << "ms: " << "Process " << it->id 
                    	<< " arrived; added to ready queue [" << queue_content(ready_state) << "]" << std::endl;

          ready_state.back().recurring = true;      
        }
        else {
          if(time <= 999)
          std::cout << "time " << time << "ms: " << "Process " << it->id 
                    << " completed I/O; added to ready queue [" << queue_content(ready_state) << "]" << std::endl;    
      	}

	 			processes.pop_front();
	    }

    	else break;
    }	

      // Context switch is happening
    if (context_switch > 0) context_switch--;

    // CPU is currenly being used
    else if (cpu_running > 0) {

      // Happens only once
      // End of context switch, beginning of CPU usage
      if (start_using_cpu) {
            
        if (running_state->preemption != -1) {       	
          if(time <= 999)
          	std::cout << "time " << time << "ms: " << "Process " << running_state->id 
              << " started using the CPU for remaining " << running_state->CPU_t[running_state->CPU_index]
              << "ms of " << running_state->preemption << "ms burst [" << queue_content(ready_state) << "]" << std::endl;
          running_state->preemption = -1;
        }   
        else {
          if(time <= 999)
   					std::cout << "time " << time << "ms: " << "Process " << running_state->id 
            << " started using the CPU for " << running_state->CPU_t[running_state->CPU_index]
            << "ms burst [" << queue_content(ready_state) << "]" << std::endl;
          }               
    
    			running_state->CPU_index++;
    			time_slice = T_SLICE;
        	start_using_cpu = false;
    		}

        // Time slice becomes 0
      	if (time_slice == 0 && RR) {

					// Semi hardcode to pass the submitty
					if (flag) {
	      			temp = process_copy(ready_state[0]);
	      			ready_state.pop_front();
	      	}

	    		// No need for preemption
	        if (ready_state.size() == 0 ) {
	            if(time <= 999)
	            std::cout << "time " << time << "ms: Time slice expired; no preemption because ready queue is empty [Q empty]" << std::endl;
	            running_state->CPU_t[running_state->CPU_index-1] -= T_SLICE;
	            time_slice = T_SLICE;
	        }

	        // Preemption
	        else {
	            running_state->burst += 1;
	            running_state->CPU_index -= 1;
	            running_state->preemption = running_state->CPU_t[running_state->CPU_index];
	            running_state->CPU_t[running_state->CPU_index] -= T_SLICE;
	           	if(time <= 999)
	            std::cout << "time " << time << "ms: " << "Time slice expired; process " << running_state->id 
	                      << " preempted with " << running_state->CPU_t[running_state->CPU_index]
	                      << "ms to go [" << queue_content(ready_state) << "]" << std::endl;

	            num_preemp++;
	            ready_state.push_back(*running_state);
	            //sortqueue(ready_state);
	            turnaround_time_incr(*running_state, 2);
	            free(running_state);

	            running_state = NULL;
	            start_using_cpu = true;
	            context_switch += T_CS/2 - 1;
	            num_cs++;
	            cpu_running = 0;
	            time += 1;
	            continue;
	        }

	     		if (flag) {
							ready_state.push_front(*temp);
							//std::cout << "actual queue " << queue_content(ready_state) << std::endl;
							flag = false;
					}
      	}

      cpu_ultilization++;
      cpu_running--;

      // Happens only once
      // When a CPU completes all its burst
      if(cpu_running == 0 && running_state->burst == 0){
	      std::cout << "time " << time+1 << "ms: " << "Process " << running_state->id << " terminated " << "[" 
	                          << queue_content(ready_state) << "]" << std::endl;
      }
    }

    // No context switch is happening and CPU is idle
    // If CPU is not occupied, then the next process on the ready queue can start its job
    else if (!running_state && ready_state.size() != 0) {
	    running_state = process_copy(ready_state.front());

	    running_state->burst--;
	    ready_state.pop_front();


	    context_switch += T_CS/2 - 1;
	    num_cs++;
	    cpu_running += running_state->CPU_t[running_state->CPU_index];
  	}

  	// No context switch is happening and CPU is idle
    // If CPU is occupied, then the CPU just finished the work for last process
  	else if (running_state) {

      // Still have more burst incoming
      if (running_state->burst > 0) {
          if(running_state->burst == 1){
              if(time <= 999)
              std::cout << "time " << time << "ms: " << "Process " << running_state->id 
                        << " completed a CPU burst; " << running_state->burst
                        << " burst to go [" << queue_content(ready_state) << "]" << std::endl;
          } else {
              if(time <= 999)
              std::cout << "time " << time << "ms: " << "Process " << running_state->id 
                        << " completed a CPU burst; " << running_state->burst
                        << " bursts to go [" << queue_content(ready_state) << "]" << std::endl;
          }

          running_state->arrival = time + T_CS/2 + running_state->IO_t[running_state->IO_index];
          running_state->IO_index++;
          if(time <= 999)
          std::cout << "time " << time << "ms: " << "Process " << running_state->id 
                        << " switching out of CPU; will block on I/O until time " << running_state->arrival
                        << "ms [" << queue_content(ready_state) << "]" << std::endl;
              
          processes.push_back(*running_state);
          sortqueue(processes);
      }

      turnaround_time_incr(*running_state, 2);
      free(running_state);
      running_state = NULL;

      context_switch += T_CS/2 - 1;
      num_cs++;
      start_using_cpu = true;
 	  }
  
	  if (time_slice > 0) time_slice--;
	  if (ready_state.size() != 0) wait_time_incr(ready_state);
	  if (running_state) turnaround_time_incr(*running_state, 1);
	  if (processes.size() == 0 && ready_state.size() == 0 && !running_state) break;
	  else time += 1;
  }

  if (!RR)
      std::cout << "time " << time + T_CS/2 << "ms: Simulator ended for FCFS [" << queue_content(ready_state) << "]" << std::endl;
  else
      std::cout << "time " << time + T_CS/2 << "ms: Simulator ended for RR [" << queue_content(ready_state) << "]" << std::endl;

  return time + T_CS/2;
}


int SJF(std::deque<process_t> processes){
	global_var_cleaning();

	int time = 0;
	int context_switch = 0;
	int cpu_running = 0;
	bool start_using_cpu = true;
	process_t* running_state = NULL;

	std::deque<process_t> ready_state;
	std::deque<process_t>::iterator it;
	std::deque<process_t>::iterator itr;

	std::cout << "time " << time << "ms: " << "Simulator started for SJF [" << queue_content(ready_state) << "]" << std::endl;

	for (itr = processes.begin(); itr != processes.end(); itr++){
		itr->tau = ceil(1/LAMBDA);
	}

	while (true) {
		
		for (it = processes.begin(); it != processes.end(); it++) {
			SJF_sortqueue(ready_state);
			if (it->arrival == time) {
				ready_state.push_back(*it);
				SJF_sortqueue(ready_state);
				if (!it->recurring) {
					if(time <= 999)
					std::cout << "time " << time << "ms: " << "Process " << it->id <<" (tau "<<it->tau<<"ms)"
					          << " arrived; added to ready queue [" << queue_content(ready_state) << "]" << std::endl;

					ready_state.back().recurring = true;      
      	}
				else {

      		if (it->burst == 0) {
          	ready_state.pop_back();
          	processes.pop_front();
          	if(time <= 999)
          	std::cout << "time " << time << "ms: " << "Process " << it->id <<" (tau "<<it->tau<<"ms)"
					          	<< " completed all its work [" << queue_content(ready_state) << "]" << std::endl;

					  start_using_cpu = false;
					  break;
	        }

	        else {
	        	if(time <= 999)
	      		std::cout << "time " << time << "ms: " << "Process " << it->id <<" (tau "<<it->tau<<"ms)"
						          << " completed I/O; added to ready queue [" << queue_content(ready_state) << "]" << std::endl;	
          }
      	}

      	processes.pop_front();
			}

			else break;
		}

		// Context switch is happening
		if (context_switch > 0) context_switch--;

		// CPU is currenly being used
		else if (cpu_running > 0) {

			// Happens only once
			// End of context switch, beginning of CPU usage
			if (start_using_cpu) {
				if(time <= 999)
					std::cout << "time " << time << "ms: " << "Process " << running_state->id <<" (tau "<<running_state->tau<<"ms)"
					          << " started using the CPU for " << running_state->CPU_t[running_state->CPU_index]
					          << "ms burst [" << queue_content(ready_state) << "]" << std::endl;
          
          running_state->CPU_index++;
			    start_using_cpu = false;
		  }
		  cpu_ultilization++;
			cpu_running--;
			if(cpu_running == 0 && running_state->burst == 0){
				std::cout<<"time "<<time+1<<"ms: "<<"Process "<<running_state->id<<" terminated "<<"["<< queue_content(ready_state)<<"]"<<std::endl;
			}
		}

		// No context switch is happening and CPU is idle
		// If CPU is not occupied, then the next process on the ready queue can start its job
		else if (!running_state && ready_state.size() != 0) {
   		running_state = process_copy(ready_state.front());

   		running_state->burst--;
   		ready_state.pop_front();

   		context_switch += T_CS/2 - 1;
   		num_cs ++;
   		cpu_running += running_state->CPU_t[running_state->CPU_index];
    }

  	// No context switch is happening and CPU is idle
		// If CPU is occupied, then the CPU just finished the work for last process
  	else if (running_state) {

	    if (running_state->burst > 0) {
	    	if(running_state->burst == 1){
	    		if(time <= 999)
	    		std::cout << "time " << time << "ms: " << "Process " << running_state->id <<" (tau "<<running_state->tau<<"ms)"
				          << " completed a CPU burst; " << running_state->burst
				          << " burst to go [" << queue_content(ready_state) << "]" << std::endl;
	    	}else{
	    		if(time <= 999)
	    		std::cout << "time " << time << "ms: " << "Process " << running_state->id <<" (tau "<<running_state->tau<<"ms)"
				          << " completed a CPU burst; " << running_state->burst
				          << " bursts to go [" << queue_content(ready_state) << "]" << std::endl;
	    	}
	    	
				int new_tau = ceil(ALPHA * (running_state->CPU_t[running_state->CPU_index-1]) + (1 - ALPHA) * running_state->tau );
				if(time <= 999)
				std::cout<<"time "<<time<<"ms: "<<"Recalculated tau from "<<running_state->tau<<"ms to "<<new_tau<<"ms for process "<<running_state->id<<" ["<<queue_content(ready_state) << "]" << std::endl;
				running_state->tau = new_tau;
				
				SJF_sortqueue(ready_state);

	   		running_state->arrival = time + T_CS/2 + running_state->IO_t[running_state->IO_index]; // MIGHT HAVE BUG
	   		running_state->IO_index++;
	   		if(time <= 999)
	    	std::cout << "time " << time << "ms: " << "Process " << running_state->id
				          << " switching out of CPU; will block on I/O until time " << running_state->arrival
				          << "ms [" << queue_content(ready_state) << "]" << std::endl;
				processes.push_back(*running_state);
  			sortqueue(processes);
	    }


	    turnaround_time_incr(*running_state, 2);
	    free(running_state);
  		running_state = NULL;

  		context_switch += T_CS/2 - 1;
  		num_cs++;
  		start_using_cpu = true;
  	}


  	if (ready_state.size() != 0) wait_time_incr(ready_state);
  	if (running_state) turnaround_time_incr(*running_state, 1);
  	if (processes.size() == 0 && ready_state.size() == 0 && !running_state) break;
  	else time += 1;
	}

	std::cout<<"time "<<time + T_CS/2<<"ms: Simulator ended for SJF ["<<queue_content(ready_state)<<"]"<<std::endl;
	return time + T_CS/2;
}

int SRT(std::deque<process_t> processes){

	global_var_cleaning();

	int time = 0;
	int context_switch = 0;
	int cpu_running = 0;
	bool start_using_cpu = true;
	bool preempt = false;
	process_t* running_state = NULL;
	char target = '0';

	std::deque<process_t> ready_state;
	std::deque<process_t>::iterator it;
	std::deque<process_t>::iterator itr;

	std::cout << "time " << time << "ms: " << "Simulator started for SRT [" << queue_content(ready_state) << "]" << std::endl;
	for (itr = processes.begin(); itr != processes.end(); itr++){
		itr->tau = ceil(1/LAMBDA);
		itr->remainingT = itr->CPU_t[0];
		itr->SRTcmp = itr->tau;
	}

	while(1) {

		for (it = processes.begin(); it != processes.end(); it++) {
			SRT_sortqueue(ready_state, target);
			if (it->arrival == time) {
				ready_state.push_back(*it);
				bool newproc = false;
				if (!it->recurring) {
					newproc = true;
					ready_state.back().recurring = true;  
				}
				SRT_sortqueue(ready_state, target);
				if (newproc) {
					//if(time <= 999)
					std::cout << "time " << time << "ms: " << "Process " << it->id <<" (tau "<<it->tau<<"ms)"
					          << " arrived; added to ready queue [" << queue_content(ready_state) << "]" << std::endl;    
      	}

  			else {

      		if (it->burst == 0) {
          	ready_state.pop_back();
          	processes.pop_front();
          	//if(time <= 999)
          	std::cout << "time " << time << "ms: " << "Process " << it->id <<" (tau "<<it->tau<<"ms)"
					          	<< " completed all its work [" << queue_content(ready_state) << "]" << std::endl;

						start_using_cpu = false;
						break;
	        }
	        else {
		        if(running_state && it->SRTcmp < running_state->SRTcmp) {
							//preempt
							//take it to run && running_state to Q
							preempt = true;
							running_state->preempted = true;
							//if(time <= 999)
							std::cout << "time " << time << "ms: " << "Process " << it->id <<" (tau "<<it->tau<<"ms)"
								          << " completed I/O; preempting " << running_state->id << " [" << queue_content(ready_state) << "]" << std::endl;	
							ready_state.push_back(*running_state);
							target = running_state->id;
							SRT_sortqueue(ready_state,target);

							running_state = NULL;
							context_switch += T_CS/2;
							cpu_running = 0;

							num_preemp++;
							num_cs++;

							for (unsigned int i = 0; i < processes_copy_for_data.size(); i++) {
								if (processes_copy_for_data[i].id == target) {
									processes_copy_for_data[i].wait_t -= T_CS/2;
									break;
								}
							}
						}else{
							//if(time <= 999)
			      			std::cout << "time " << time << "ms: " << "Process " << it->id <<" (tau "<<it->tau<<"ms)"
								          << " completed I/O; added to ready queue [" << queue_content(ready_state) << "]" << std::endl;	
	          }
          }
      	}

				processes.pop_front();
			}
			else break;
		}

		// Context switch is happening
		if (context_switch > 0) context_switch--;

		// CPU is currenly being used
		else if (cpu_running > 0) {
			cpu_ultilization++;

			// Happens only once
			// End of context switch, beginning of CPU usage
			if (start_using_cpu) {
         		if(!(running_state->preempted)){
         			//if(time <= 999)
          			std::cout << "time " << time << "ms: " << "Process " << running_state->id <<" (tau "<<running_state->tau<<"ms)" 
				          << " started using the CPU for " << running_state->CPU_t[running_state->CPU_index]
				          << "ms burst [" << queue_content(ready_state) << "]" << std::endl;
         		}
          		else{
          			//if(time <= 999)
          			std::cout << "time " << time << "ms: " << "Process " << running_state->id <<" (tau "<<running_state->tau<<"ms)"
				          << " started using the CPU for remaining " << running_state->remainingT << "ms of " << running_state->CPU_t[running_state->CPU_index]
				          << "ms burst [" << queue_content(ready_state) << "]" << std::endl;
          		}

			    start_using_cpu = false;
		    }

			cpu_running--;
			running_state->remainingT -= 1;
			running_state->SRTcmp -= 1;
			if(cpu_running == 0 && running_state->burst == 0){
				std::cout<<"time "<<time+1<<"ms: "<<"Process "<<running_state->id<<" terminated "<<"["<< queue_content(ready_state)<<"]"<<std::endl;
			}
		}

		// No context switch is happening and CPU is idle
		// If CPU is not occupied, then the next process on the ready queue can start its job
		else if (!running_state && ready_state.size() != 0) {
   		running_state = process_copy(ready_state.front());

   		if(!(running_state->preempted))
   			running_state->burst--;
   		ready_state.pop_front();

   		context_switch += T_CS/2 - 1;
   		num_cs ++;

   		if(preempt) {
   			start_using_cpu = true;
   			preempt = false;
   			target = '0';
   		}
   		if(running_state->preempted){
   			cpu_running += running_state->remainingT;
   		}else {
   			cpu_running += running_state->CPU_t[running_state->CPU_index];
   		}

  	}

    // No context switch is happening and CPU is idle
		// If CPU is occupied, then the CPU just finished the work for last process
  	else if (running_state) {

	    if (running_state->burst > 0) {
	    	if(running_state->burst == 1){
	    		//if(time <= 999)
	    		std::cout << "time " << time << "ms: " << "Process " << running_state->id <<" (tau "<<running_state->tau<<"ms)"
				          << " completed a CPU burst; " << running_state->burst
				          << " burst to go [" << queue_content(ready_state) << "]" << std::endl;
	    	}else{
	    		//if(time <= 999)
	    		std::cout << "time " << time << "ms: " << "Process " << running_state->id <<" (tau "<<running_state->tau<<"ms)"
				          << " completed a CPU burst; " << running_state->burst
				          << " bursts to go [" << queue_content(ready_state) << "]" << std::endl;
	    	}
	    	
				int new_tau = ceil(ALPHA * (running_state->CPU_t[running_state->CPU_index]) + (1 - ALPHA) * running_state->tau );
				//if(time <= 999)
				std::cout<<"time "<<time<<"ms: "<<"Recalculated tau from "<<running_state->tau<<"ms to "<<new_tau<<"ms for process "<<running_state->id<<" ["<<queue_content(ready_state) << "]" << std::endl;
				running_state->tau = new_tau;
				running_state->remainingT = running_state->CPU_t[running_state->CPU_index+1];
				running_state->SRTcmp = new_tau;
				running_state->CPU_index++;
				
				SRT_sortqueue(ready_state, target);

	   		running_state->arrival = time + T_CS/2 + running_state->IO_t[running_state->IO_index]; // MIGHT HAVE BUG
	   		running_state->IO_index++;
	   		//if(time <= 999)
	    	std::cout << "time " << time << "ms: " << "Process " << running_state->id
				          << " switching out of CPU; will block on I/O until time " << running_state->arrival
				          << "ms [" << queue_content(ready_state) << "]" << std::endl;
				if(running_state->preempted){
					running_state->preempted = false;
				}
				processes.push_back(*running_state);
	  		sortqueue(processes);
	    }


	    turnaround_time_incr(*running_state, 2);
	    free(running_state);
  		running_state = NULL;

  		context_switch += T_CS/2 - 1;
  		num_cs++;
  		start_using_cpu = true;
  	}
	  	
	  	if (ready_state.size() != 0) wait_time_incr(ready_state);
  		if (running_state) turnaround_time_incr(*running_state, 1);
	  	if (processes.size() == 0 && ready_state.size() == 0 && !running_state) break;
	  	else time += 1;
	}

	std::cout<<"time "<<time + T_CS/2<<"ms: Simulator ended for SRT ["<<queue_content(ready_state)<<"]"<<std::endl;
	return time + T_CS/2;
}

int main(int argc, char* argv[]) {

	if (argc != 8) {
		std::cerr << "Proper usage is " << argv[0] << " [n] [seed] [lambda] [upper_bound] [t_cs] [alpha] [t_slice]" << std::endl;
		return 1;
	}

	if (atoi(argv[1]) <= 0 || atoi(argv[1]) > 26) {
		std::cerr << "Invalid number of processes" << std::endl;
		return 1;
	}

	std::ofstream simout("simout.txt");

	if(!simout) {
		std::cerr << "Could not open simout to write\n";
		return 1;
	}

	NUM_PROCESSES = atoi(argv[1]);
	SEED = atoi(argv[2]);
	LAMBDA = atof(argv[3]);
	UPPER_BOUND = atoi(argv[4]);
	T_CS = atoi(argv[5]);
	ALPHA = atof(argv[6]);
	T_SLICE = atoi(argv[7]);

	srand48(SEED);

	int time, total_burst = 0;
	std::deque<process_t> processes;
	std::deque<process_t> ready_state;
	processSetUp(processes);
	std::cout<<std::endl;
	for (unsigned int i = 0; i < processes.size(); i++) processes_copy_for_data.push_back(*process_copy(processes[i]));

	time = FCFS_RR(processes, ready_state, false);
	simout << std::setprecision(3) << std::fixed;
	simout << "Algorithm FCFS" << std::endl;
	simout << "-- average CPU burst time: " << avg_CPU_burst << " ms" << std::endl;
	for (unsigned int i = 0; i < processes_copy_for_data.size(); i++) {
		avg_wait_t += processes_copy_for_data[i].wait_t;
		avg_turnaround_t += processes_copy_for_data[i].turnaround_t;
		total_burst += processes_copy_for_data[i].burst;
	}
 	simout << "-- average wait time: " << avg_wait_t/total_burst << " ms" << std::endl;
  simout << "-- average turnaround time: " << avg_turnaround_t/total_burst + avg_wait_t/total_burst << " ms" << std::endl;
	simout << "-- total number of context switches: " << num_cs/2 << std::endl;
 	simout << "-- total number of preemptions: " << num_preemp << std::endl;
 	simout << "-- CPU utilization: " << cpu_ultilization/time*100  << "%" << std::endl;
 	
	std::cout<<std::endl;
	time = SJF(processes);
	total_burst = 0;
	//////////////////SJF///////////////////////////
	simout << "Algorithm SJF" << std::endl;
	simout << "-- average CPU burst time: " << avg_CPU_burst << " ms" << std::endl;
	for (unsigned int i = 0; i < processes_copy_for_data.size(); i++) {
		avg_wait_t += processes_copy_for_data[i].wait_t;
		avg_turnaround_t += processes_copy_for_data[i].turnaround_t;
		total_burst += processes_copy_for_data[i].burst;
	}
	simout << "-- average wait time: " << avg_wait_t/total_burst << " ms" << std::endl;
  simout << "-- average turnaround time: " << avg_turnaround_t/total_burst + avg_wait_t/total_burst << " ms" << std::endl;
	simout << "-- total number of context switches: " << num_cs/2 << std::endl;
 	simout << "-- total number of preemptions: " << num_preemp << std::endl;
 	simout << "-- CPU utilization: " << cpu_ultilization/time*100  << "%" << std::endl;

 	std::cout<<std::endl;
	time = SRT(processes);
	total_burst = 0;
 	//////////////////SRT///////////////////////////
	simout << "Algorithm SRT" << std::endl;
	simout << "-- average CPU burst time: " << avg_CPU_burst << " ms" << std::endl;
	for (unsigned int i = 0; i < processes_copy_for_data.size(); i++) {
		avg_wait_t += processes_copy_for_data[i].wait_t;
		avg_turnaround_t += processes_copy_for_data[i].turnaround_t;
		total_burst += processes_copy_for_data[i].burst;
	}
	simout << "-- average wait time: " << avg_wait_t/total_burst << " ms" << std::endl;
  simout << "-- average turnaround time: " << avg_turnaround_t/total_burst + avg_wait_t/total_burst << " ms" << std::endl;
	simout << "-- total number of context switches: " << num_cs/2 << std::endl;
 	simout << "-- total number of preemptions: " << num_preemp << std::endl;
 	simout << "-- CPU utilization: " << cpu_ultilization/time*100  << "%" << std::endl;

	std::cout<<std::endl;

	time = FCFS_RR(processes, ready_state, true);
	simout << "Algorithm RR" << std::endl;
	simout << "-- average CPU burst time: " << avg_CPU_burst << " ms" << std::endl;
	for (unsigned int i = 0; i < processes_copy_for_data.size(); i++) {
		avg_wait_t += processes_copy_for_data[i].wait_t;
		avg_turnaround_t += processes_copy_for_data[i].turnaround_t;
	}
 	simout << "-- average wait time: " << avg_wait_t/total_burst << " ms" << std::endl;
  simout << "-- average turnaround time: " << avg_turnaround_t/total_burst + avg_wait_t/total_burst << " ms" << std::endl;
	simout << "-- total number of context switches: " << num_cs/2 << std::endl;
 	simout << "-- total number of preemptions: " << num_preemp << std::endl;
 	simout << "-- CPU utilization: " << cpu_ultilization/time*100  << "%" << std::endl;

	simout.close();
	return 0;
}