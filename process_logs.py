import os, sys
from pathlib import Path

ADJUST = dict(
    #wspace=1, hspace=0.5,
    left=0.175,
    top=0.99,
    right=0.99,
    bottom=0.1,
)

def parse(file):
    n = None
    ret = {}
    # hack to reuse some previous data before benchmark is finished (not used)
    # if "dense_" in file:
    #     ret[24] = 286.78, 37.19
    #     ret[25] = 753.53, 111.58
    #     ret[26] = 2391.58, 334.73
    #     ret[27] = 6519.28, 1004.19
    #     for n in list(ret):
    #         sec, GB = ret[n]
    #         kb = GB * 1e9 / 1024
    #         ret[n] = sec, kb
    #print(file)
    for line in open(file):
        line = line.strip()
        if not line:
            n = None
        elif "density = " in line:
            n = int(line.split()[2])
        elif line.startswith("elapsed RAM"):
            kb = int(line.split()[-2]) / 1024
        elif line.startswith("elapsed TIME"):
            sec = float(line.split()[-2]) / 10**9
            # print("n=", n, sec, "seconds", kb, "kb", "= %.3f" % (kb / 2**20), "GiB")
            ret[n] = (sec, kb)

        # elif line.startswith("Maximum resident set"):
    # print()
    return ret

fnames = """
logs/benchmark_dense_100.log
logs/benchmark_sparse_25.log
logs/benchmark_sparse_50.log
logs/benchmark_sparse_99.log
""".split()

data = [parse(f) for f in fnames]


# for n in (16, 20, 21, 23, 24, 27, 29, 31):
rng = list(range(16, 32))
for n in rng:
    print(n)
    for ret in data:
        if n not in ret:
            print("& - & -")
        else:
            sec, kb = ret[n]
            kb = float(kb)

            mins = sec / 60
            hs = mins / 60
            if sec < 0.1:
                s = r"%.2f \texttt{s}" % sec
            elif sec < 3:
                s = r"%.1f \texttt{s}" % sec
            elif sec < 100:
                s = r"%.0f \texttt{s}" % sec
            elif sec < 1000:
                s = r"%.1f \texttt{m}" % (sec / 60)
            else:
                s = r"%.1f \texttt{h}" % (sec / 3600)

            mb = kb / 2**10
            gb = mb / 2**10
            tb = gb / 2**10
            if kb < 100:
                k = "%.0f KiB" % kb

            elif mb < 5:
                k = "%.1f MiB" % mb
            elif mb < 100:
                k = "%.0f MiB" % mb

            elif gb < 5:
                k = "%.1f GiB" % gb
            elif gb < 100:
                k = "%.0f GiB" % gb

            elif tb < 5:
                k = "%.1f TiB" % tb
            elif tb < 100:
                k = "%.0f TiB" % tb
            else:
                raise
            print("&", k, "&", s)
    if n != rng[-1]:
        print(r'\\')
    print()



import os
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.pyplot import savefig

#matplotlib.rcParams['mathtext.fontset'] = 'custom'
# matplotlib.rcParams['mathtext.rm'] = 'Bitstream Vera Sans'
# matplotlib.rcParams['mathtext.it'] = 'Bitstream Vera Sans:italic'
# matplotlib.rcParams['mathtext.cursive'] = 'Bitstream Vera Sans:italic'
# matplotlib.rcParams['mathtext.bf'] = 'Bitstream Vera Sans:bold'
#matplotlib.rcParams['font.family'] = 'serif'
# matplotlib.rcParams['mathtext.fontset'] = 'stix'
# matplotlib.rcParams['font.family'] = 'STIXGeneral'
#matplotlib.rc('font',**{'family':'sans-serif','sans-serif':['Bitstream Vera Sans']})

fnames = {
"SparseQMC(d=0.25)": "logs/benchmark_sparse_25.log",
"SparseQMC(d=0.50)": "logs/benchmark_sparse_50.log",
"SparseQMC(d=0.75)": "logs/benchmark_sparse_75.log",
#"SparseQMC(d=0.55)": "logs/benchmark_sparse_55.log",
"SparseQMC(d=0.99)": "logs/benchmark_sparse_99.log",
# "SparseQMC(d=1.00)": "logs/benchmark_sparse_100.log",
# "DenseQMC(d=any)": "logs/benchmark_dense_99.log",
"DenseQMC(d=1.00)": "logs/benchmark_dense_100.log",
}
MARKERS = [">", "v", "o", "d", "^", "s", "x"]
data = {title: parse(f) for title, f in fnames.items()}


plt.rc("legend", loc="lower right")
scale = 0.6
plt.rcParams["figure.figsize"] = 9*scale, 7.5*scale



plt.rc('grid', linestyle="--", color='lightgray')
fig, ax = plt.subplots()
plt.xticks(range(8, 32, 2))
plt.yscale('log', base=3)
#plt.yticks([3**i for i in range(-5, 30)])
skip = [0, 4, 7, -6]
yticks = [3**i for i in range(-25, 30) if i not in skip] + [3**i for i in skip]
ylabs = [r"$3^{%d}$" % i for i in range(-25, 30) if i not in skip] + ["" for i in skip]
yticks = []
ylabs = []
plt.grid()

def add_time_mark(secs, lab):
    yticks.append(secs)
    ylabs.append(lab)
    ax.plot([8, 31], [secs, secs], color="black", alpha=0.3, linestyle="--")

add_time_mark(1e-3, "1 ms")
add_time_mark(1, "1 second")
add_time_mark(60, "1 minute")
add_time_mark(3600, "1 hour")
# add_time_mark(3600*10, "10 hours")

ax.set_yticks(yticks)
ax.set_yticklabels(ylabs)


markers = MARKERS[:]
for title, ret in data.items():
    ns = list(sorted(ret))
    vals = [ret[n][0] for n in ns]
    #vals = [val if val > 0.01 else 0.00 for val in vals]
    # todel = []
    # for i in range(len(ns)):
    #     if vals[i] < 0.02:
    #         todel.append(i)
    # for i in todel[::-1]:
    #     del vals[i]
    #     del ns[i]

    #print(ns)
    #print(vals)
    ax.plot(ns, vals, label=title, marker=markers.pop(), markersize=5)


ax.set_xlabel(r'Number of variables $n$')
ax.set_ylabel('Time (log scale)')
ax.legend()
#plt.subplots_adjust(wspace=1,hspace=0.5,left=0.1,top=0.9,right=0.9,bottom=0.1)
plt.subplots_adjust(**ADJUST)

fig.savefig("figures/time_dense_sparse.png", dpi=600)
#os.system("convert figures/time_dense_sparse.png -trim figures/time_dense_sparse_trim.png")








plt.rc('grid', linestyle="--", color='lightgray')
fig, ax = plt.subplots()
plt.xticks(range(8, 32, 2))
plt.yscale('log', base=3)
#plt.yticks([3**i for i in range(-5, 30)])
skip = [0,6,7,12,13,-6]
yticks = [3**i for i in range(-25, 30) if i not in skip] + [3**i for i in skip]
ylabs = [r"$3^{%d}$" % i for i in range(-25, 30) if i not in skip] + ["" for i in skip]
yticks = []
ylabs = []
plt.grid()

def add_time_mark(secs, lab, addline=True):
    yticks.append(secs)
    ylabs.append(lab)
    if addline:
        ax.plot([8, 31], [secs, secs], color="black", alpha=0.3, linestyle="--")

add_time_mark(1/1024.0, "1 KiB")
#add_time_mark(2**-5.0, "32 KiB", 0)
add_time_mark(2**00, "1 MiB")
#add_time_mark(2**5, "32 MiB", 0)
add_time_mark(2**10, "1 GiB")
#add_time_mark(2**15, "32 GiB", 0)
add_time_mark(2**10 * 10, "10 GiB", 0)
add_time_mark(2**20, "1 TiB")

ax.set_yticks(yticks)
ax.set_yticklabels(ylabs)

markers = MARKERS[:]
for title, ret in data.items():
    ns = list(sorted(ret))
    vals = [ret[n][1]/1024 for n in ns]
    #vals = [val if val > 0.01 else 0.00 for val in vals]
    # todel = []
    # for i in range(len(ns)):
    #     if vals[i] < 0.02:
    #         todel.append(i)
    # for i in todel[::-1]:
    #     del vals[i]
    #     del ns[i]

    #print(ns)
    #print(vals)
    ax.plot(ns, vals, label=title, marker=markers.pop())


ax.set_xlabel(r'Number of variables $n$')
ax.set_ylabel(r'Memory (log scale)')
ax.legend()

plt.subplots_adjust(**ADJUST)

fig.savefig("figures/mem_dense_sparse.png", dpi=600)
# os.system("convert figures/mem_dense_sparse.png -trim figures/mem_dense_sparse_trim.png")
