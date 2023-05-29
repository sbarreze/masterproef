import sys
import matplotlib.pyplot as plt

title = input()
input() # customers

cont = True
all_f = []

while cont:

    try:
        descriptor = input()
        print(descriptor)
    except EOFError:
        cont = False
        break

    # Parse input data into separate lists
    fitness = list(map(float, input().split()))
    epochs = [i for i in range(len(fitness))]

    # Create and display the graph
    plt.plot(epochs, fitness, label=descriptor)

    all_f += fitness

smallest = min([e for e in all_f if e > 0])
print(smallest)
biggest = max(all_f)

plt.ylim(bottom=smallest - 0.05 * smallest, top=biggest + 0.05 * biggest)
plt.xlabel('Epoch')
plt.ylabel(f'Fitness: {title}')
plt.legend()
plt.show()
