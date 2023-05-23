# Parallel solver for the frequency assignment problem for a system of wireless mics
It's the 15-418 Parallel Computer Architecture and Programming final project!

This project implements a parallel algorithm to find a set of compatible radio frequencies for a large set of wireless mics. It uses ISPC and a shared memory model and is run on GHC’s 8-core Intel Core i7 machines. There are multiple implementations used to benchmark performance: a sequential, parallel single core, and parallel multicore program. The parallel multicore version achieves 21x speedup compared to the sequential version for checking compatibility. The conclusion is to rewrite this with OpenMP.

The “Frequency Assignment Problem” reduces to a vertex coloring problem, an NP-complete problem. This problem is complex because adding a wireless mic into a system creates interference, called intermodulation (IM) products, with every combination of every other mic in the system throughout the available frequency range. As a result, solvers need to perform an exhaustive search or some other computationally expensive approximation. We try to speed this search up by running it in parallel.

See the [webpage](https://shannoding.github.io/15418-wireless-frequency/) and [paper](https://shannoding.github.io/15418-wireless-frequency/paper.pdf).
