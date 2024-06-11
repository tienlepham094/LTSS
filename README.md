https://github.com/hoangdzung/LR_MPI/tree/master </br >
https://github.com/Lehmannhen/MPI-Dijkstra/tree/master

**Update cỳ anh**
### chạy test_train_gen.py trước khi chạy linear.c
```
python3 test_train_gen.py
```
2 tham số lần lượt là số lượng mẫu(n_samples) và số chiều %%Update%%

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

### Chạy matrix_gen.py trước khi chạy dijsktra.c
```
python3 random_matrix.py
```

Tham số đầu vào lần lượt là kích cỡ ma trận

```
mpicc dijsktra.c -o dijsktra
mpirun -np 3 dijsktra < input2.txt
(chu y so process va size cua ma tran)

```
Mount nfs 
```
sudo mount 172.20.10.9:/home/openmpi/Desktop/sharedfolder /home/openmpi/Desktop/sharedfolder
```
run 
```
mpirun --hostfile /etc/hosts -np 5 ./outputfile
```