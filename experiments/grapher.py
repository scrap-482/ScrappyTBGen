from matplotlib import pyplot as plt
import pandas as pd

def main():
    fname = 'retrograde-analysis/results/retro_analysis_mpi_3man'
    df = pd.read_csv(fname)

    title = 'MPI 3-man Retrograde Analysis Speedup'
    
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

    plt.savefig('retro_analysis_mpi_3man_speedup.png')
    plt.cla()
    
    title = 'MPI 3-man Retrograde Analysis Runtime'
    plt.plot(xs, ys, '-o')
    plt.title(title)
    plt.ylabel(ylabel)

    plt.savefig('retro_analysis_mpi_3man_runtime.png')

if __name__ == '__main__':
    main()
