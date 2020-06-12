import subprocess
import shlex
import time
import multiprocessing
import os
import signal

from subprocess import check_output

from concurrent.futures import TimeoutError
from pebble import ProcessPool, ProcessExpired

# Dicionario de PROCESSOS - tem que ser com o Manager pois são processos distintos
manager = multiprocessing.Manager()
pid_dict = manager.dict()
pid_dict["teste"] = 99999999999


def kill_espresso_process(pla_name):
    return True


def update_pid_dict(nome, pid):
    global pid_dict
    pid_dict.update({nome: pid})


# func para gerar todos os nomes dos arquivos PLAs do contest
def gera_arquivo_todos_os_plas():
    with open("MaioresPLA.txt", "w") as arquivo:
        plas = range(100)
        for i in plas:
            arquivo.write("ex{:02d}.total.pla\n".format(i))
            arquivo.write("ex{:02d}.train.pla\n".format(i))
            arquivo.write("ex{:02d}.valid.pla\n".format(i))
