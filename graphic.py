import numpy as np
import matplotlib.pyplot as plt


matr = np.loadtxt('data.txt')
plt.plot(matr[:, 0], matr[:, 1], '-r')
plt.title('График зависимости накладных расходов от размера блока')
plt.xlabel('Размер блока')
plt.ylabel('Расход')
plt.show()