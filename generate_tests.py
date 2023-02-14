import random
from binteger import Bin

random_id = 0

def generate_random(n, density):
    global random_id
    random_id += 1
    random.seed("random%d" % random_id)

    lst = list(range(2**n))
    take = random.sample(lst, int(density*2**n / 100.0 + 0.5))
    random.shuffle(take)  # input does not have to be sorted

    fd = open("tests/dnf_n%d_density%d_id%d.in" % (n, density, random_id), "w")
    fc = open("tests/cnf_n%d_density%d_id%d.in" % (n, density, random_id), "w")
    print("d", n, file=fd)
    print("c", n, file=fc)
    for x in take:
        x = Bin(x, n)
        print(x.str, file=fd)
        print(x.str, file=fc)
    fd.close()
    fc.close()

def generate_sparse_dnf(n, number, active, dir="tests"):
    global random_id
    random_id += 1
    random.seed("sparse%d" % random_id)

    bits = [2**i for i in range(n)]
    mask = sum(random.sample(bits, active))
    take = list({random.randrange(2**n) & mask for _ in range(number)})
    random.shuffle(take)  # input does not have to be sorted

    fd = open(dir + "/dnf_n%d_number%d_active%d_id%d.in" % (n, number, active, random_id), "w")
    print("d", n, file=fd)
    for x in take:
        x = Bin(x, n)
        print(x.str, file=fd)
    fd.close()

generate_random(1, 0)
generate_random(1, 100)

generate_random(2, 0)
generate_random(2, 100)

generate_random(8, 0)
for i in range(10):
    generate_random(8, 10)
    generate_random(8, 25)
    generate_random(8, 50)
    generate_random(8, 75)
    generate_random(8, 80)
    generate_random(8, 99)
generate_random(8, 100)

generate_random(12, 0)
for i in range(10):
    generate_random(12, 10)
    generate_random(12, 25)
    generate_random(12, 50)
    generate_random(12, 75)
    generate_random(12, 80)
    generate_random(12, 99)
generate_random(12, 100)

generate_random(16, 1)
generate_random(16, 10)

for i in range(10):
    for active in (1, 2, 4, 5, 10, 15, 20):
        generate_sparse_dnf(20, 0, active)
        generate_sparse_dnf(20, 1, active)
        generate_sparse_dnf(20, 2, active)
        generate_sparse_dnf(20, 100, active)
        generate_sparse_dnf(20, 1000, active)


for i in range(3):
    for n in (30, 31, 32, 33, 34, 40, 50, 60, 61, 62):
        for active in (1, 2, 4, 5, 10):
            generate_sparse_dnf(n, 0, active, dir="tests_ext")
            generate_sparse_dnf(n, 1, active, dir="tests_ext")
            generate_sparse_dnf(n, 2, active, dir="tests_ext")
            generate_sparse_dnf(n, 100, active, dir="tests_ext")
            generate_sparse_dnf(n, 1000, active, dir="tests_ext")
