import subprocess
from subprocess import check_output
from multiprocessing import Pool
from time import time


def best_aig(number):
    print("%02d" % number)
    process = check_output(["python3.8",
                            "IWLS2020.py",
                            "-i",
                            "PLA_files/ex%02d.total.pla" % number,
                            "-o",
                            "Contest_AIGs/ex%02d.aig" % number,
                            "--persist",
                            "--verbose"], stderr=subprocess.DEVNULL)
    with open("Contest_AIGs_INFO/ex%02d.txt" % number, "w") as file:
        file.write(process.decode())


if __name__ == "__main__":
    tempo_inicial = time()
    with Pool(2) as p:
        p.map(best_aig, [0, 1])
    print("Runtime: " + str(time()-tempo_inicial))
