# OpenMP Epidemic Simulation

![Language](https://img.shields.io/badge/language-C-blue.svg)
![Technology](https://img.shields.io/badge/technology-OpenMP-green.svg)
![Status](https://img.shields.io/badge/status-Complete-success)

A high-performance agent-based simulation modeling the spread of a contagious disease within a population. The project demonstrates parallel computing techniques using **OpenMP**, implementing both **Data Parallelism** (Loop-level) and **Task Parallelism** strategies to optimize execution time on multi-core architectures.

## 📋 Project Overview

The simulation models a closed population moving within a 2D grid. The logic follows a discrete time-step approach where agents interact based on spatial proximity.

### Core Mechanics:
* **Movement:** Agents move with specific vectors (Direction + Amplitude), reflecting off the simulation borders.
* **Transmission:** A Susceptible agent becomes Infected if they share coordinates with an Infected agent.
* **State Machine:** Agents transition through states: `Susceptible` -> `Infected` (for fixed duration) -> `Immune` (for fixed duration) -> `Susceptible`.

## 🚀 Technical Implementation

The simulation utilizes a **Grid-Based Spatial Partitioning** approach to optimize collision detection, reducing the interaction complexity from $O(N^2)$ to $O(N)$.

### Parallelization Strategies

The project implements two distinct parallel architectures to compare scheduling overhead and load balancing:

#### Version 1: Loop Parallelism (`#pragma omp parallel for`)
* Utilizes implicit work-sharing constructs.
* Distributes the agent population dynamically across threads.
* **Synchronization:** Uses implicit barriers at the end of parallel loops to ensure all agents complete a time step before the next begins.

#### Version 2: Task Parallelism (`#pragma omp task`)
* Implements **Manual Domain Decomposition**.
* The population is logically partitioned into chunks.
* Explicit tasks are spawned for each chunk, allowing for more flexible scheduling logic.
* **Synchronization:** Uses `#pragma omp taskwait` to synchronize the simulation steps.

## 🛠️ Build & Run

### Prerequisites
* GCC Compiler with OpenMP support
* CMake (optional, for build automation)

### Compilation
```bash
gcc -fopenmp main.c -o epidemic_sim
```

### Usage
```bash
./epidemic_sim <TOTAL_TIME> <INPUT_FILE> <NUM_THREADS>
```
* **TOTAL_TIME:** Number of simulation steps (e.g., 100).
* **INPUT_FILE:** Configuration file containing population size and initial states.
* **NUM_THREADS:** Number of parallel threads to spawn.

### Example
```bash
./epidemic_sim 200 input_data.txt 4
```
## 📊 Performance Analysis

Performance measurements were conducted on a multi-core system, varying the population size from 10,000 to 500,000 agents. The results demonstrate significant scalability, particularly for larger datasets where the computational load justifies the parallelization overhead.

### Runtime Comparison (in Seconds)

| Population | Serial | Parallel V1 (16 Th) | Parallel V2 (16 Th) | Speedup (Approx) |
| :--- | :--- | :--- | :--- | :--- |
| **10,000** | 0.005s | 0.015s | 0.007s | *Slowdown (Overhead)* |
| **50,000** | 0.170s | 0.038s | 0.032s | ~5.3x |
| **100,000** | 2.502s | 0.152s | 0.147s | ~16.5x |
| **500,000** | 12.390s | 0.537s | 0.529s | **~23x** |

> **Note:** Super-linear speedup observed in high-load scenarios is likely due to better cache locality per core when distributing the grid data.

### Key Observations

1.  **Scalability:** The application scales efficiently with the problem size. For a population of **500k**, execution time dropped from **12.39s (Serial)** to **~0.53s (16 Threads)**.
2.  **Task vs. Loop Parallelism:**
    * **Version 2 (Tasks)** consistently outperformed Version 1 (Loop) by a small margin (e.g., 0.529s vs 0.537s at max load). This suggests that the explicit task decomposition handled the workload slightly more efficiently than the implicit `omp parallel for` scheduling.
3.  **Parallel Overhead:**
    * For small inputs (10k agents), the parallel versions (especially with 16 threads) were slower than the serial version or low-thread versions. This highlights the **synchronization overhead** cost: spawning threads and managing tasks takes more time than the actual simulation logic when the dataset is trivial.

### Conclusion
The simulation achieves optimal performance when the dataset is large enough to saturate the available cores, demonstrating the effective use of OpenMP for high-performance computing tasks.

## 📂 Project Structure

* `main.c`: Core logic containing Serial, Parallel V1, and Parallel V2 implementations.
* `Task`: Original project requirements and constraints.
* `CMakeLists.txt`: Build configuration.

## ⚠️ Disclaimer
This project is an academic simulation designed to demonstrate parallel programming concepts (Race Conditions, Synchronization, Load Balancing) and is not intended for epidemiological decision-making.

## 👤 Author

**Denis-Răzvan Radu**
Computer Engineering Student, Politehnica University of Timișoara