import matplotlib.pyplot as plt

# Your data
data = [
    [200000, 1.638, 1.241],
    [240000, 2.152, 1.653],
    [280000, 2.376, 1.770],
    [320000, 2.870, 2.232],
    [360000, 3.182, 2.656],
    [400000, 4.013, 3.638]
]

y = [row[0] for row in data]
x1 = [row[1] for row in data]
x2 = [row[2] for row in data]

# Create the plot
plt.figure(figsize=(8, 6))  # Adjust figure size if needed

plt.plot(x1, y, label="T_w_com", marker='o', linestyle='-')  # Plot x1 vs. y
plt.plot(x2, y, label="T_wo_com", marker='s', linestyle='-')  # Plot x2 vs. y

plt.xlabel("T")
plt.ylabel("n_samples")
plt.title("Plot ot vs n_samples")
plt.legend()
plt.grid(True)  # Add grid lines
plt.show()