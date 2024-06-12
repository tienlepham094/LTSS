import random

def generate_matrix(size):
  """Tạo ma trận ngẫu nhiên và lưu vào file txt.

  Args:
    size: Kích thước của ma trận (size x size).
  """
  matrix = []
  for i in range(size):
    row = []
    for j in range(size):
        if i == j:
          weight = 0  # Đường chéo chính bằng 0
        else:
          # Chọn ngẫu nhiên giữa 1 và 2, nếu là 1 thì trọng số từ 1-30, 
          # nếu là 2 thì trọng số là 999
          choice = random.choices([1,2], weights=(50,50)) 
        
          weight = random.randint(1, 10) if choice[0] == 1 else 999  
        row.append(weight)
    matrix.append(row)
    
  for i in range(size):
    for j in range(i+1,size):
        if matrix[i][j] != 999:
            matrix[j][i] = 999
        else:
            matrix[j][i]= random.randint(1, 10)
            
  with open("matrix.txt", "w") as f:
    print(size, file=f)
    for row in matrix:
      f.write(" ".join(str(x) for x in row) + "\n")

#nhap kich thuoc ma tran
size = int(input("Nhập kích thước ma trận: "))

# tao ma tran va luu vao file
generate_matrix(size)
print(f"Đã tạo ma trận kích thước {size}x{size} vào file 'matrix.txt'")