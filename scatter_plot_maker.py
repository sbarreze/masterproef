import seaborn as sns
import matplotlib.pyplot as plt
import sys

fileName = input()
input() # Customer list, not needed for this

# Read data from file
data = []
for line in sys.stdin:
    alpha, delta, fitness = line.strip().split('\t')
    data.append([float(alpha), float(delta), float(fitness)])

# Filter out invalid fitness values
data = [[alpha, delta, fitness] for alpha, delta, fitness in data if fitness != 0]

# Convert data to a pandas DataFrame
import pandas as pd
df = pd.DataFrame(data, columns=['Alpha', 'Delta', 'Fitnes'])

# Reshape data into a grid
grid = df.pivot(index='Alpha', columns='Delta', values='Fitnes')

# Create heatmap plot
sns.set(font_scale=1.2)
annot_kws = {'fontsize': 10, 'fontweight': 'bold', 'color': 'black'}  # Annotation settings
cmap = sns.light_palette("seagreen", reverse=True, as_cmap=True) # Color palette
sns.heatmap(grid, cmap=cmap, annot=True, fmt='.1f', cbar_kws={'label': 'Fitnes'}, annot_kws=annot_kws)

xas = 'mutatieProb'
yas = 'kruisingProb'
plt.xlabel(xas)
plt.ylabel(yas)
plt.show()