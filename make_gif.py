import matplotlib.pyplot as plt
import imageio

problem_descriptor = input()
customers_str = [tp_str for tp_str in input().split()]
customers = []
for customer in customers_str:
    customers.append(tuple(int(x) for x in customer.split(',')))
iteration_nr = 0
cont = True

while cont:

    try:
        iteration_descriptor = input()
    except EOFError:
        cont = False
        break 

    fig = plt.figure(figsize=(10, 10))
    fig.suptitle(iteration_descriptor)

    routes = [ int(i) for i in input().strip().split() ][1:-1]
    route = [ 0 ]
    for cust in routes:
        route += [cust]
        if cust == 0:
            x = [ customers[i][0] for i in route ]
            y = [ customers[i][1] for i in route ]

            plt.plot(x, y)
            route = [0]

    route += [0]
    x = [ customers[i][0] for i in route ]
    y = [ customers[i][1] for i in route ]

    # PLOTTING THE DATA FOR EACH ALGORITHM
    plt.plot(x, y)

    plt.plot(customers[0][0], customers[0][1], markersize=20)
    plt.text(customers[0][0]-0.5, customers[0][1]+0.3, "DEPOT")

    plt.savefig(f'./gif/img_{iteration_nr}.png', transparent=False, facecolor= 'white')

    plt.close()

    iteration_nr += 1

frames = []
for t in range(iteration_nr):
    img = imageio.v3.imread(f'./gif/img_{t}.png')
    frames.append(img)

imageio.mimsave('./examplePSA.gif', frames, fps=2, loop=1)