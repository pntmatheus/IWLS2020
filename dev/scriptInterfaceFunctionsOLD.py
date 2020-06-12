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

def update_pid_dict(nome, pid):
    global pid_dict
    pid_dict.update({nome : pid})

# func para gerar todos os nomes dos arquivos PLAs do contest
def gera_arquivo_todos_os_plas():
    with open("MaioresPLA.txt", "w") as arquivo:
        plas = range(100)
        for i in plas:
            arquivo.write("ex{:02d}.total.pla\n".format(i))
            arquivo.write("ex{:02d}.train.pla\n".format(i))
            arquivo.write("ex{:02d}.valid.pla\n".format(i))

def retorna_nome_espresso(nome_arquivo):
    nome_espresso = nome_arquivo.replace("\n", "")
    nome_espresso = nome_espresso.replace(".pla", "")
    nome_espresso = "%s-ESPRESSO-NORMAL.pla" % nome_espresso
    return nome_espresso

def gera_espresso_pla_wrapper(args):
    return gera_espresso_pla(*args)

def gera_espresso_pla(arquivo_entrada, arquivo_saida):
    tempo_inicial = time.time()
    # subprocess -> usado para executar comando via python
    # shlex      -> usado para "quebrar" a string em <comando> e <argumentos>
    espresso_cmd = subprocess.Popen(shlex.split('./espresso ' + arquivo_entrada),
                   stdout=subprocess.PIPE)
    update_pid_dict(arquivo_entrada.replace("PLA_files/",""), espresso_cmd.pid)
    # executa o comando ./espresso
    # O retorno do "communicate" eh: [0] -> stdout e [1] -> sterr
    saida = espresso_cmd.communicate()

    # abre o arquivo
    arquivo = open(arquivo_saida, "wb")

    # escreve a saida do espresso
    arquivo.write(saida[0])

    #fecha o arquivo
    arquivo.close()

    tempo_final = time.time() - tempo_inicial

    return tempo_final

def gera_abc_aig(arquivo_pla, nome_aig):
    #
    # Comandos no ABC
    # read_pla -->
    # strash   -->
    # balance  -->
    # write    -->
    abc_cmd = subprocess.Popen(shlex.split("./abc -c \'read_pla %s;"
                                                      'strash;'
                                                      'balance;'                                              
                                                      'balance;'
                                                      'balance;'
                                                      'balance;'
                                                      'balance;'                                              
                                                      'balance;'
                                                      'balance;'
                                                      'balance;'
                                                      'balance;'                                              
                                                      'balance;'
                                                      'balance;'
                                                      'balance;'
                                                      "write %s\'" % (arquivo_pla,nome_aig)),
                                           stdout=subprocess.PIPE)
    # O retorno desta funcao sera [0] = stdout e [1] = sterr (erro)
    return abc_cmd.communicate()

def abc_mltest_aig(arquivo_aig, arquivo_pla):

    def pega_valor_ps(saida_abc, string_busca, espaco_campo):
        posicao_inicial = saida_abc.find(string_busca)
        posicao_final = posicao_inicial + len(string_busca)
        valor_campo = int(saida_abc[posicao_final:posicao_final + espaco_campo])
        return valor_campo

    abc_cmd_aig = subprocess.Popen(shlex.split('./abc -c \'&read %s;'
                                                          '&ps;'
                                                          '&mltest %s\'' % (arquivo_aig, arquivo_pla)),
              stdout=subprocess.PIPE)

    #executa o comando e guarda na variavel
    saida_abc = abc_cmd_aig.communicate()

    #print(saida_abc[0])



    # Pegar numero de INPUTS no AIG
    in_outs = b"i/o ="
    posicao_inicial = saida_abc[0].find(in_outs)
    posicao_final = posicao_inicial + len(in_outs)
    numero_ins = int(saida_abc[0][posicao_final:posicao_final+7])
    numero_outs = int(saida_abc[0][posicao_final+9:posicao_final+16])

    # Pegar numero de ANDs no AIG
    numero_ands = int(pega_valor_ps(saida_abc[0], b"and =", 8))
    # Pegar a profundidade do AIG
    numero_levs = int(pega_valor_ps(saida_abc[0], b"lev =", 5))
    # Pegar a quantidade de Erros do AIG
    numero_erros = int(pega_valor_ps(saida_abc[0], b"Errors =", 7))
    # Pegar a quantidade de Acertos do AIG
    numero_acertos = int(pega_valor_ps(saida_abc[0], b"Correct =", 7))
    # Pegar a quantidade de vetores que saem 0 -- NAIVE GUESS -- (&mltest)
    numero_naive = int(pega_valor_ps(saida_abc[0], b"Naive guess =", 7))
    # Calcula o total de vetores
    total = numero_acertos + numero_erros
    # Calcula o percentual de acertos
    percentual_acerto = numero_acertos * 100 / total

    output = {
        "inputs"    : numero_ins,
        "outputs"   : numero_outs,
        "ands"      : numero_ands,
        "levels"    : numero_levs,
        "erros"     : numero_erros,
        "acertos"   : numero_acertos,
        "naive"     : numero_naive,
        "per_acerto": percentual_acerto
    }
    return output