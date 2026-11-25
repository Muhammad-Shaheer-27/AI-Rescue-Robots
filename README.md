# AI Rescue Robots — Autonomous Disaster-Response Simulation

## Project Overview

A C++ Data Structures project that simulates autonomous rescue robots responding to disaster-related requests in a dynamic environment.

The simulation models locations, road connections, rescue requests, robot capabilities, battery levels, and dynamic events such as road blockages and station failures.

## Implemented Features

- Custom graph representation using an adjacency list.
- Pointer-based linked lists and custom data structures.
- Shortest-path routing using Dijkstra's algorithm.
- Rescue request prioritization and robot assignment.
- Battery management and charging.
- Dynamic event processing.
- Simulation logs and performance statistics.

## Project Structure

- `src/` — C++ source and header files.
- `data/` — Scenario input files.
- `tests/` — Test cases and testing documentation.
- `output/` — Generated simulation results.
- `DS Project Report.pdf` — Final project report.

## Requirements

- A C++ compiler supporting C++11 or later.
- No STL containers or external pathfinding and scheduling libraries are used.

## Compilation

Run the following command from the project root directory:

```bash
g++ -std=c++11 src/*.cpp -o rescue_sim
```

## Execution

Run the program:

```bash
./rescue_sim
```

Alternatively, provide a scenario file as a command-line argument:

```bash
./rescue_sim data/scenario_provided.txt
```

If no file is provided, the program asks for a scenario path. Press Enter to load `data/scenario_provided.txt` by default.

## Input Format

Scenario files are stored in the `data/` directory and contain the following sections:

- `SCENARIO` — Scenario name.
- `PARAM` — Simulation parameters.
- `LOCATIONS` — Locations and their types.
- `CONNECTIONS` — Roads, costs, and statuses.
- `ROBOTS` — Robot information and capabilities.
- `REQUESTS` — Rescue request information.
- `EVENTS` — Scheduled events.
- `END` — Marks the end of the scenario file.

Blank lines and lines beginning with `#` are ignored.

## Output Format

After the simulation finishes, the program saves two files in the `output/` directory:

- `<scenario_name>_logs.csv` — Chronological simulation event log.
- `<scenario_name>_statistics.txt` — Final simulation statistics.

The program also displays simulation information and statistics in the terminal.

**Note:** The `output/` directory must exist before running the simulation.

## Usage

After loading a scenario, the program displays a menu with the following options:

1. Run the complete simulation automatically.
2. Advance the simulation by one time step.
3. Display the current simulation status.
4. Display the current environment.
5. Display robot information.
6. Display rescue request information.
7. Exit.

## Status

Implementation and testing completed. Refer to `report.pdf` for the project design, algorithms, complexity analysis, performance evaluation, and results.