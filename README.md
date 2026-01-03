# Conway's Game of Life (C + ncurses)

This project implements Conway's Game of Life using the C programming language and the `ncurses` library for the Text User Interface (TUI).

## Prerequisites

- C Compiler
- `ncurses` library 

## Usage

1. **Compile**:
   ```bash
   make
   ```

2. **Run**:
   ```bash
   ./game_of_life <input_file>
   ```

   Replace `<input_file>` with the path to your starting pattern file. Classic patterns are available in the `boards/` directory

3. **Controls**:
- **Space**: Start/Stop simulation.
- **Q**: Quit.
