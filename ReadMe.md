# Ocean Life Simulation

## Overview

The Ocean Life Simulation is a toroidal-based simulation where different entities interact in a two-dimensional grid that represents the ocean. The simulation provides a dynamic ecosystem featuring stones, reefs, prey, predators, and apex predators, each with unique behaviors and lifecycles. The simulation is visually displayed in the terminal, showing interactions such as hunting, reproduction, and transformations.

---

## Features

### Entity Types

1. **Empty Cell (`  `)**:
   - Represents an unoccupied space in the ocean.

2. **Stone (`S`)**:
   - Slowly transforms into a reef after a certain number of ticks.

3. **Reef (`R`)**:
   - Provides shelter to prey from predators. Reverts back to a stone after a while.

4. **Prey (`~`)**:
   - Moves to avoid predators and reproduces when conditions are favorable.

5. **Predator (`P`)**:
   - Hunts prey. Reproduces and avoids apex predators. Dies if it becomes too hungry or old.

6. **Apex Predator (`A`)**:
   - The top predator. Can eat both prey and predators when extremely hungry. Moves faster when starving.

### Simulation Features

- **Toroidal World**:
  - The grid wraps around edges; creatures exiting one side reappear on the opposite side.

- **Random Storm Events**:
  - Occasionally, a storm clears a small region, adding unpredictability to the ecosystem.

- **Extended Lifespans**:
  - All entities have longer lifespans to create a more dynamic simulation.

- **Statistics**:
  - Displays the count of each entity type and the current simulation iteration.

- **Configurable Grid Size**:
  - Users can set the dimensions of the ocean via command-line arguments.

---

## Requirements

- **C++ Compiler**:
  - Compatible with C++11 or higher.

- **Operating System**:
  - Works on Windows, Linux, and macOS.

---

## Building and Running

### Linux/macOS

1. Compile:
   ```bash
   g++ -std=c++11 -Wall -o ocean_sim index.cpp
   ```
2. Run:
   ```bash
   ./ocean_sim <rows> <columns>
   ```
   Example:
   ```bash
   ./ocean_sim 30 40
   ```

### Windows (MinGW)

1. Compile:
   ```bash
   g++ -std=c++11 -Wall -o ocean_sim.exe index.cpp
   ```
2. Run:
   ```bash
   ocean_sim.exe <rows> <columns>
   ```
   Example:
   ```bash
   ocean_sim.exe 30 40
   ```
3. If using `cmd.exe`, enable UTF-8 mode with:
   ```cmd
   chcp 65001
   ```

---

## How to Play

1. **Simulation Start**:
   - The ocean initializes with a random distribution of entities.

2. **Display**:
   - Symbols represent different entities in the terminal.

3. **Dynamic Interaction**:
   - Observe hunting, reproduction, and transformation behaviors.

4. **Simulation End**:
   - The simulation stops after a fixed number of iterations or if the ocean stabilizes with no significant changes.

---

## Code Structure

- **`index.cpp`**:
  - Contains the main simulation logic, entity definitions, and action implementations.

- **`README.md`**:
  - Documentation for the project.

---

## Customization

1. **Lifespans**:
   - Modify `maxAge` in the `Prey`, `Predator`, or `ApexPredator` classes to adjust lifespans.

2. **Reproduction**:
   - Adjust `reproduceCountdown` logic for entity reproduction intervals.

3. **Grid Size**:
   - Pass custom grid dimensions via command-line arguments.

4. **Storm Frequency**:
   - Change the probability of storms in the `Ocean::run()` method.

---

## Example Output

```
Iteration: 50  (No change counter: 0)
----- Ocean Statistics -----
Empty: 1024
Stone: 12
Reef: 20
Prey: 45
Predator: 15
ApexPredator: 5

Ocean Grid:
S  S  ~  P  A  ~  R  R  P  P  
~  ~     P  A  A     ~  ~  P  
R  S     P        ~        ~  
```

---

## License

This project is open-source and free for personal or educational use. For significant modifications or contributions, consider attributing the original author.

