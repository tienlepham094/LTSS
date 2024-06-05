import matplotlib.pyplot as plt

# Your data
data = [
    [2000, 0.016, 0.014],
    [4000, 0.094, 0.088],
    [6000, 0.171, 0.163],
    [8000, 0.386, 0.371],
    [16000, 1.333, 1.314],
    [20000, 2.061, 2.038],
    [40000, 7.463, 7.406],
    [60000, 19.091, 19.011],
    [10000, 50.123, 50.014]
]

y = [row[0] for row in data]
x1 = [row[1] for row in data]
x2 = [row[2] for row in data]

# Create the plot
plt.figure(figsize=(8, 6))  # Adjust figure size if needed

plt.plot(x1, y, label="x1", marker='o', linestyle='-')  # Plot x1 vs. y
#plt.plot(x2, y, label="x2", marker='s', linestyle='-')  # Plot x2 vs. y

plt.xlabel("x1 and x2")
plt.ylabel("y")
plt.title("Plot of y vs. x1 and x2")
plt.legend()
plt.grid(True)  # Add grid lines
plt.show()