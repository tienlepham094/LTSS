import numpy as np
import random

def generate_linear_data(n_samples, num_attributes, noise_scale=0.5):
  """Generates a linear dataset with random features and labels.

  Args:
    n_samples: The number of samples to generate.
    num_attributes: The number of attributes (features).
    noise_scale: The scale of random noise added to the labels.

  Returns:
    A tuple containing:
      - X: A numpy array of shape (n_samples, num_attributes) representing features.
      - Y: A numpy array of shape (n_samples,) representing labels.
  """

  weights = np.random.randn(num_attributes)

  X = np.random.rand(n_samples, num_attributes)

  X += np.random.randn(n_samples, num_attributes) * noise_scale  
  X = np.round(X, 1)
  
  Y = np.dot(X, weights) + np.random.randn(n_samples) * noise_scale
  Y = np.round(Y, 1)

  return X, Y

def save_data(filename, X, Y):
  """Saves data to a file in the format expected by the C code.

  Args:
    filename: The name of the file to save the data.
    X: The feature matrix.
    Y: The label vector.
  """

  with open(filename, 'w') as f:
    f.write(f"{X.shape[0]} {X.shape[1]+1}\n")
    for i in range(X.shape[0]):
      for j in range(X.shape[1]):
        f.write(f"{X[i, j]} ")
      f.write(f"{Y[i]}\n")

if __name__ == "__main__":
  n_samples = int(input("so luong mau:"))
  num_attributes = int(input("so chieu du lieu:"))
  noise_scale = 0.5  # Adjust noise level as needed

  # Generate training data
  X, Y = generate_linear_data(n_samples, num_attributes, noise_scale)

  # Split into train and test (80% train, 20% test)
  train_size = int(0.8 * n_samples)
  X_train = X[:train_size]
  Y_train = Y[:train_size]
  X_test = X[train_size:]
  Y_test = Y[train_size:]

  # Save the train data
  save_data("linear.train", X_train, Y_train)

  # Save the test data
  save_data("linear.test", X_test, Y_test)

  print("Da tao du lieu va luu vao file 'linear.train' va 'linear.test'")