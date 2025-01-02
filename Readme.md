# OS Scheduler

This project simulates a CPU scheduler, a critical component of an operating system responsible for determining the execution order of scheduled processes. It implements the following scheduling algorithms and integrates the buddy system for memory allocation.

## Features

### Scheduling Algorithms
1. **Shortest Job First (SJF):** Selects the process with the shortest execution time.
2. **Preemptive Highest Priority First (PHPF):** Always selects the process with the highest priority.
3. **Round Robin (RR):** Executes processes in a cyclic order, with a fixed time quantum.
4. **Multilevel Feedback Queue (MLFQ):** Dynamically adjusts the scheduling based on process behavior and priority.

### Customization
- **Algorithm Selection:** Users can choose the desired scheduling algorithm by modifying parameters in the `Makefile`.
- **Quantum Adjustment:** For algorithms requiring a time quantum (e.g., Round Robin), the quantum value can be customized in the `Makefile`.

## Usage

1. Clone the repository:
   ```bash
   git clone <repository_url>
   cd os_scheduler
   ```

2. Edit the `Makefile` to select the scheduling algorithm and adjust parameters like the quantum value.

3. Compile the project:
   ```bash
   make
   ```

4. Run the scheduler:
   ```bash
   make run
   ```

## Implementation Details

- **Buddy System:** Used for efficient memory allocation, minimizing fragmentation.
- **Interactive User Input:** Provides flexibility to test and observe different scheduling strategies.

## Contributing
Contributions are welcome! Feel free to fork the repository and submit a pull request.


