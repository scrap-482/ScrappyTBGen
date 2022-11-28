from matplotlib import pyplot as plt
import pandas as pd

def main():
    fname = 'checkmate-identification/results/checkmate_omp'
    df = pd.read_csv(fname)

    title = 'OMP 4-man Checkmate Identification Speedup'
    
    xs = df.iloc[:, 0]
    ys = df.iloc[:, 1]

    xlabel = df.columns[0]
    ylabel = df.columns[1]
    
    #speedup
    speedups = [ys[0] / y for y in ys]

    plt.plot(xs, speedups, '-o')
    plt.xlabel(xlabel)
    plt.title(title)
    plt.ylabel('Speedup')

    plt.savefig('checkmate_omp_speedup.png')
    plt.cla()
    
    title = 'OMP 4-man Checkmate Identification Runtime'
    plt.plot(xs, ys, '-o')
    plt.title(title)
    plt.ylabel(ylabel)

    plt.savefig('checkmate_omp_runtime.png')

if __name__ == '__main__':
    main()
