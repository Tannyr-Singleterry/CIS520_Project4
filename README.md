To run our 3 implementations of Project 4 please follow the next instructions.
___

After logging into and transfering the files to beocat go into the project directory

```cd CIS520_Project4```

Then, go into the pthread directory with

```cd 3way-pthread```

Then, to schedule the job do

```sbatch run_pthread.slurm```

After that the job will be scheduled and you can watch the output live with


```tail -f perf_pthread_<job_id>.out```

The final output will be in ```perf_pthread_<job_id>.out```, and the raw data in ```perf_data_pthread_<job_id>.csv```, and any errors in ```perf_pthread_<job_id>.err```

___

Next to schedule and run the mpi implementation we follow a similar process

If you are in the pthread directory you can do 

```cd ../3way-mpi```

Or if you are project root directory you can do 

```cd 3way-mpi```

Then, to schedule the job do

```sbatch run_mpi.slurm```

After that the job will be scheduled and you can watch the output live with

```tail -f perf_mpi_<job_id>.out```

The final output will be in ```perf_mpi_<job_id>.out```, and the raw data in ```perf_data_mpi_<job_id>.csv```, and any errors in ```perf_mpi_<job_id>.err```

___

Next to schedule and run the openMP implementation we follow a similar process

If you are in the mpi directory you can do 

```cd ../3way-openMP```

Or if you are project root directory you can do 

```cd 3way-openMP```

Then, to schedule the job do

```sbatch run_openmp.slurm```

After that the job will be scheduled and you can watch the output live with

```tail -f perf_openmp_<job_id>.out```

The final output will be in ```perf_openmp_<job_id>.out```, and the raw data in ```perf_data_openmp_<job_id>.csv```, and any errors in ```perf_openmp_<job_id>.err```
