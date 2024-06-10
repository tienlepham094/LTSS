import matplotlib.pyplot as plt
import sys
import matplotlib.pyplot as plt

def read_data(file_name):
    n_values = []
    t_w_com = []
    t_wo_com = []

    with open(file_name, 'r') as file:
        # Skip the header
        next(file)

        # Read the data
        for line in file:
            if line.strip():  # Skip empty lines
                n, t_w, t_wo = line.split(',')
                n_values.append(int(n.strip()))
                t_w_com.append(float(t_w.strip()))
                t_wo_com.append(float(t_wo.strip()))

    return n_values, t_w_com, t_wo_com

# Read the filenames from the command line arguments
if len(sys.argv) != 3:
    print("Usage: python plot_data_two_files.py <input_file1> <input_file2>")
    sys.exit(1)

input_file1 = sys.argv[1]
input_file2 = sys.argv[2]

# Read data from the files
n_samples1, t_w_com1, t_wo_com1 = read_data(input_file1)
n_samples2, t_w_com2, t_wo_com2 = read_data(input_file2)

# Create the plots
plt.figure(figsize=(14, 6))

# First subplot for the first file
plt.subplot(1, 2, 1)
plt.plot(t_w_com1, n_samples1, label="T_w_com", marker='o', linestyle='-')
plt.plot(t_wo_com1, n_samples1, label="T_wo_com", marker='s', linestyle='-')
plt.xlabel("Time (T)")
plt.ylabel("Number of Samples (n_samples)")
plt.title("Plot of Time vs. Number of Samples")
plt.legend()
plt.grid(True)

# Second subplot for the second file
plt.subplot(1, 2, 2)
plt.plot(t_w_com2, n_samples2, label="T_w_com", marker='o', linestyle='-')
plt.plot(t_wo_com2, n_samples2, label="T_wo_com", marker='s', linestyle='-')
plt.xlabel("Time (T)")
plt.ylabel("Number of CPUs (ncpus)")
plt.title("Plot of Time vs. Number of CPUs")
plt.legend()
plt.grid(True)

# Show the plots
plt.tight_layout()
plt.show()