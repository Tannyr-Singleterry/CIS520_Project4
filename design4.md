## pthread software architecture
The software architecture for the pthreads approach to this project starts with coordinating thread or the main thread. It reads lines in batches of 4096 at a time then delegates the work statically among n worker threads to compute the data. The pthreads are created all at once and stored in a pool and are awakened by a broadcast, instead of creating the threads when needed. This is intended to reduce the amount of thread creation overhead. A generation counter works as a version state variable so workers can continue when a new batch is ready. Finally, synchronization is handled using mutex locks and condition variablies which aids with ordering and safer communication between threads.

## pthread performance analysis
### Charts
![Thread count used and how it relates to time spent](design4Charts/perf_data_pthread_thread_count.pdf)

![Memory allowed and how it relates to time spent](design4Charts/perf_data_pthread_memory.pdf)
### Analysis
Based on the data we can see that 1 thread took the absolute longest while 4 threads took the least amount of total time on average. With using more threads the expected outcome should be that as you increase threads the time it takes to process data goes down. However this is not the case. It suggests that there is some unaccounted for overhead that comes with creating and managing these threads.

