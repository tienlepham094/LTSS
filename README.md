https://github.com/hoangdzung/LR_MPI/tree/master </br >
https://github.com/Lehmannhen/MPI-Dijkstra/tree/master

**Run linear.c**
```
mpicc linear.c -o linear -lm
mpirun linear
```

**Run dijsktra.c**
```
mpicc dijsktra.c -o dijsktra
mpirun dijsktra < input.txt
```