# Classic Tetris Game in C

A terminal-based implementation of the classic Tetris game written in C, inspired by Alexey Pajitnov's original Tetris.



## Features

- Classic Tetris gameplay with all 7 standard tetrominos
- Increasing difficulty levels
- Score tracking with high score system
- Game controls including rotation, movement, hard and soft drop
- Pause functionality
- Level selection
- Simple ASCII graphics interface

## Requirements

- Windows OS (uses Windows-specific console functions)
- C compiler (gcc recommended)
- make (for using the Makefile)

## How to Compile and Run

### Using Make

```bash
# Navigate to the tetris directory
cd tetris

# Compile the game
make

# Run the game
./tetris
```

### Manual Compilation

```bash
# Navigate to the tetris directory
cd tetris

# Compile with gcc
gcc -Wall -o tetris tetris.c

# Run the game
./tetris
```

## Game Controls

- **Left/Right Arrow:** Move the tetromino left or right
- **Up Arrow:** Rotate the tetromino
- **Down Arrow:** Soft drop (move down faster)
- **Space Bar:** Hard drop (instantly place the tetromino)
- **P:** Pause the game
- **Q:** Quit the game

## Scoring System

- 1 line cleared: 40 × level points
- 2 lines cleared: 100 × level points
- 3 lines cleared: 300 × level points
- 4 lines cleared: 1200 × level points
- Soft drop: 1 point per row
- Hard drop: 2 points per row

## Educational Notes

This project was created as part of a beginner's tutorial series on C programming fundamentals. It demonstrates:

- Structs and enums
- Arrays (including multi-dimensional arrays)
- Functions and control flow
- File I/O for high score persistence
- Game loop implementation
- Console manipulation
- User input handling

Check out the full tutorial video on [RedaHustlzTV YouTube channel](https://www.youtube.com/c/RedaHustlzTV).

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Inspired by Alexey Pajitnov's original Tetris game
- Created for educational purposes as part of RedaHustlzTV's C programming tutorials