# You Spin Me Round Robin

This lab implements round robin scheduling in C for a given workload and quantum length. 

## Building
To build, navigate to the lab2 directory and run:
```shell
make
```

## Running
The arguments to run this file are a text of processes, with their arrival time, start time, and burst time. And the quantum length for a time slice.
```shell
./rr processes.txt [quantum_length]
```

Here is an example of what the output might look like after running the scheduler:
```shell
Average waiting time: 7.00
Average response time: 2.75
```
These results are based on the provided processes.txt file and using a quantum length of 3.


## Cleaning up
To clean up the executable and any other binary files created during the build process, run:
```shell
make clean
```

## Testing
To run the provided test cases run:
```shell
python3 -m unittest
```

