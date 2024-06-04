import random

def generate_linear_data(n_samples, data_dim, filename):
  """Tạo dữ liệu ngẫu nhiên cho bài toán hồi quy tuyến tính và lưu vào file.

  Args:
    n_samples: Số lượng mẫu dữ liệu.
    data_dim: Số chiều dữ liệu (số lượng features).
    filename: Tên file để lưu dữ liệu.
  """

  with open(filename, "w") as f:
    f.write(f"{n_samples} {data_dim}\n") 

    for _ in range(n_samples):
      features = []
      for _ in range(data_dim - 1):
        # Tạo feature ngẫu nhiên khác 0
        feature = 0
        while feature == 0:  # Lặp lại cho đến khi feature khác 0
          feature = round(random.uniform(-2, 2), 1)
        features.append(feature)
      #label = round(sum(features) + random.gauss(0, 1), 2) # Tạo nhãn (tổng features + nhiễu Gaussian)
      label = random.randint(1, 3)    
      f.write(" ".join(str(x) for x in features) + f" {label}\n")  # Ghi dữ liệu vào file

n_samples = int(input("so luogn mau:"))
data_dim = int(input("so chieu du lieu:"))

# Tạo file linear.train
generate_linear_data(n_samples, data_dim, "linear.train")
print(f"Đã tạo file 'linear.train' với {n_samples} mẫu, {data_dim - 1} features.")

# Tạo file linear.test (sử dụng 20% dữ liệu cho kiểm tra)
n_samples_test = int(n_samples) #*0.2
generate_linear_data(n_samples_test, data_dim, "linear.test")
print(f"Đã tạo file 'linear.test' với {n_samples_test} mẫu, {data_dim - 1} features.")