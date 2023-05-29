#!/usr/bin/python

import matplotlib
matplotlib.use('Qt5Agg')
import matplotlib.pyplot as plt

# General info
nr_of_algorithms = int(input())

while True:

	try:
		problem_descriptor = input()
	except EOFError:
		exit(-1)
	fig, axs = plt.subplots(1, nr_of_algorithms)
	fig.suptitle(problem_descriptor)

	try:
		customers_str = [tp_str for tp_str in input().split()]
	except EOFError:
		exit(-1)
	customers = []
	for customer in customers_str:
		customers.append(tuple(float(x) for x in customer.split(',')))

	# Plot each outcome of each algorithm
	for j in range(nr_of_algorithms):

		routes = [ int(i) for i in input().strip().split() ]
		algorithm_descriptor = input()

		route = [ 0 ]
		for cust in routes:
			route += [cust]
			if cust == 0:
				x = [ customers[i][0] for i in route ]
				y = [ customers[i][1] for i in route ]

				axs[j].plot(x, y)
				route = [0]

		route += [0]
		x = [ customers[i][0] for i in route ]
		y = [ customers[i][1] for i in route ]

		# PLOTTING THE DATA FOR EACH ALGORITHM
		axs[j].plot(x, y)

		axs[j].plot(customers[0][0], customers[0][1], markersize=20)
		axs[j].text(customers[0][0]-0.5, customers[0][1]+0.3, "DEPOT")
		axs[j].set_title(algorithm_descriptor)

	plt.show()