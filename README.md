# Conway's Game of Life

This project is an implementation of Conway's Game of Life in C. It uses the `ncurses` library for the Text User Interface (TUI).
It uses **Multiprocessing**, **Shared Memory**, and **Semaphores** to separate the simulation logic from the user interface.

## Prerequisites

* **GCC Compiler**
* **Make** build system
* **Ncurses library** (`libncurses`)

## Usage

### 1. Compile
To build the project, simply run:
```bash
make
```

### 2. Run
To run the application, use:
```bash
./game_of_life <input_file>
```

Replace `<input_file>` with the path to your starting pattern file. Classic patterns are available in the `boards/` directory

### 3. Controls
- **Space**: Start/Stop simulation.
- **Q**: Quit.
- **R**: Reset to initial state.
- **P**: Play/Pause simulation.
- **Arrow Keys**: Move cursor.
- **Enter**: Toggle cell at cursor position.
- **S**: Save current state to file.
- **-/+**: Adjust simulation speed.


## System Programming Architecture

To ensure high performance and responsiveness, the application is split into two independent processes that communicate via Inter-Process Communication (IPC) mechanisms.

### 1. Multiprocessing (`fork`)
The application uses the `fork()` system call to create two distinct processes:
* **Parent Process (UI)**: Responsible for rendering the grid to the terminal and handling user input. It remains responsive even if the simulation speed is slow.
* **Child Process (Engine)**: Responsible for the heavy calculation of the next generation of cells. It runs in the background, independent of the UI refresh rate.

### 2. Shared Memory (`shm_open`, `mmap`)
**POSIX Shared Memory** is used to create a memory region accessible by both the UI and the Engine.

**Memory Layout:**
The shared memory segment is structured efficiently to contain:
1.  **Control Header (`SharedState`)**: Contains atomic variables like `is_paused`, `simulation_speed`, and `current_buffer_index`.
2.  **Grid Buffers**: Three flat arrays of booleans are allocated immediately after the header:
    * **Buffer A**: The current state of the board.
    * **Buffer B**: The next state (used for double buffering).
    * **Initial Buffer**: A copy of the starting state (used for the Reset function).

### 3. Synchronization (Semaphores)
A **POSIX Named Semaphore** (`sem_open`) is used to synchronize access to the shared memory, acting as a Mutex to solve this:
* **Lock**: Before the Engine writes to the grid, or the UI reads from it, they must acquire the semaphore.
* **Unlock**: Once the operation is done, the semaphore is released.
* It ensures the grid is never read while being written to, preventing visual artifacts.
