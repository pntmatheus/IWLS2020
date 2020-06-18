import subprocess
from subprocess import check_output
from multiprocessing import Pool
from multiprocessing import set_start_method
from time import time


def best_aig(number):
    print("%02d" % number)
    process = check_output(["python3.8",
                            "IWLS2020.py",
                            "-i",
                            "PLA_files/ex%02d.total.pla" % number,
                            "-o",
                            "Contest_AIGs/ex%02d.aig" % number,
                            "--mltest",
                            "iwls_competition_0323/ex%02d.train.pla" % number,
                            "iwls_competition_0323/ex%02d.valid.pla" % number,
                            "--dont-clean-tmp",
                            "--supress",
                            "--verbose"], stderr=subprocess.PIPE)

    with open("Contest_AIGs_INFO/ex%02d.txt" % number, "w") as file:
        file.write(process.decode())


if __name__ == "__main__":
    set_start_method("spawn")
    tempo_inicial = time()
    with Pool(processes=50) as pool:
        #pool.map(best_aig, range(10))
        pool.map(best_aig, range(100))
        #print(pool.map(best_aig, [0,1]))
    print("Runtime: " + str(time()-tempo_inicial))
