# Sprintz revised
Implementation of compression algorithms for IoT applications.

### Sprintz by Blalock
Original algorithm was invented by Blalock et al. and is described here: https://arxiv.org/abs/1808.02515. Blalock also delivered source code: https://github.com/dblalock/sprintz.

### Features
Repository contains IoT intended, C implementations of:
- Sprintz
- Sprintz-Huff (novel variant)
- Fire-Rice (novel variant)
- Sprintz-tANS (novel variant)

### Repository
`main` branch contains C library with compression and decompression functions implemented, while `tests` branch contains almost the same code, but rewritten as RiotOs RTOS (https://github.com/RIOT-OS/RIOT) application module. 

### Embedded system
Library was tested on *b-l072z-lrwan1 (ARM Cortex-M0+)* and *SLWSTK6221a (ARM Cortex-M4)* single-board microcontrollers, as module of RiotOs application.
