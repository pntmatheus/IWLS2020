import time

from concurrent.futures import TimeoutError
from pebble import ProcessPool, ProcessExpired

from scriptCommonFunctions import *
from scriptEspressoFunctions import *
from scriptABCFunctions import *

#from scriptInterfaceFunctions import *

if __name__ == '__main__':
    tempo_inicial = time.time()
    with open("ListaPLA.txt", "r") as maiores:
        for pla in maiores:
            pla = pla.replace("\n", "")
            caminho = "PLA_files/%s" % pla
            timeOUT = 4
            with ProcessPool(max_workers=1) as pool:
                try:
                    future = pool.schedule(gera_espresso_pla, [caminho, retorna_nome_espresso(pla)], timeout=timeOUT)
                    print("O PLA %s levou %f " % (pla, future.result()))
                    with open("ateh60segundos.txt", "w") as file:
                        file.write("%s#######%f\n" % (pla, future.result()))
                except TimeoutError as error:
                    #print(pid_dict)
                    #print("Aqui no Timeout o PID do %s eh %s " % (pla, pid_dict[pla]))
                    os.kill(pid_dict[pla], signal.SIGTERM)
                    print("O PLA %s levou mais que o TIMEOUT que estah definido como %d segundos" % (pla,error.args[1]))
                except ProcessExpired as error:
                    print("%s. Exit code: %d" % (error, error.exitcode))
                except Exception as error:
                    print("function raised %s" % error)
                    print(error.traceback)  # Python's traceback of remote process


    print("Tempo final %f " % (time.time() - tempo_inicial))