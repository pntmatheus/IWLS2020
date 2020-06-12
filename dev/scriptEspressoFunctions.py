from subprocess import Popen, TimeoutExpired
import shlex
import time
import re

from scriptCommonFunctions import *


def le_pla(arquivo_pla):
    with open(arquivo_pla, "r") as arquivo:
        pla = arquivo.read().encode()

    pla_obj = pla_obj_factory_old(pla)

    return pla_obj



def pla_obj_factory_old(pla):
    # Eh necessario decodificar pq o pla eh uma saida binaria
    pla_decoded = pla.decode()

    # Regex patterns
    in_pat = re.compile('\.i(.*?)\\n')
    out_pat = re.compile('\.o(.*?)\\n')
    type_pat = re.compile('\.type(.*?)\\n')
    termos_pat = re.compile('\.p(.*?)\\n')
    coment_pat = re.compile('#(.*?)\\n')
    pulou_linha_pat = re.compile('(.*?)\\n')
    so_termos = re.compile('[^-\d\n\s].*')

    # Remover comentarios (se existirem!!)
    pla_sem_comentario = re.sub(coment_pat, "", pla_decoded)

    qt_ins = re.search(in_pat, pla_sem_comentario).group(1)
    qt_outs = re.search(out_pat, pla_sem_comentario).group(1)
    qt_termos = re.search(termos_pat, pla_sem_comentario).group(1)
    termos = re.findall(pulou_linha_pat, pla_sem_comentario)

    termos_on_set = list()
    termos_off_set = list()

    pla_type = re.search(type_pat, pla_sem_comentario)
    if pla_type is not None:
        pla_type = pla_type.group(1)
        pla_type = pla_type.replace(" ", "")

    qt_ins = int(qt_ins)
    qt_outs = int(qt_outs)
    qt_termos = int(qt_termos)

    lista_exclusao = list()

    for termo in termos:
        if bool(re.match(so_termos, termo)):
            lista_exclusao.append(termo)

    for termo in lista_exclusao:
        termos.remove(termo)

    for termo in termos:
        if " 1" in termo:
            termos_on_set.append(termo)
        else:
            termos_off_set.append(termo)

    pla_obj = {"qt_ins":            qt_ins,
               "qt_outs":           qt_outs,
               "type":              pla_type,
               "qt_termos":         qt_termos,
               "termos":            termos,
               "termos_on_set":     termos_on_set,
               "termos_off_set":    termos_off_set}

    return pla_obj


def cria_arquivo_pla(pla_obj, nome_arquivo):

    with open(nome_arquivo, "w") as arquivo:
        arquivo.write(".i %d\n" % pla_obj["qt_ins"])
        arquivo.write(".o %d\n" % pla_obj["qt_outs"])
        arquivo.write(".p %d\n" % pla_obj["qt_termos"])
        if pla_obj["type"] is not None:
            arquivo.write(".type %s\n" % pla_obj["type"])
        for termo in pla_obj["termos"]:
            arquivo.write("%s\n" % termo)
        arquivo.write(".e\n")

    return nome_arquivo


def retorna_nome_espresso(nome_arquivo):
    nome_espresso = nome_arquivo.replace("\n", "")
    nome_espresso = nome_espresso.replace(".pla", "")
    nome_espresso = "%s-ESPRESSO-NORMAL.pla" % nome_espresso
    return nome_espresso


def gera_espresso_pla_wrapper(args):
    return gera_espresso_pla(*args)


def gera_espresso_pla(arquivo_entrada, arquivo_saida, cmds=False, gera_arquivo=False, timeout=False):
    tempo_inicial = time.time()

    if not cmds:
        comando = "./espresso %s" % arquivo_entrada
    else:
        cmds = list()
        if gera_arquivo:
            cmds.append("-t")
        comando = "./espresso %s%s" % ("".join(["%s " % i for i in cmds]), arquivo_entrada)

    # subprocess -> usado para executar comando via python
    # shlex      -> usado para "quebrar" a string em <comando> e <argumentos>
    espresso_cmd = subprocess.Popen(shlex.split(comando),
                                    stdout=subprocess.PIPE)
    if not timeout:
        # executa o comando ./espresso
        # O retorno do "communicate" eh: [0] -> stdout e [1] -> sterr
        saida = espresso_cmd.communicate()
    else:
        try:
            saida = espresso_cmd.communicate(timeout=timeout)
        except TimeoutExpired:
            espresso_cmd.kill()
            outs, errs = espresso_cmd.communicate()
            return None

    if gera_arquivo:
        # abre o arquivo
        with open(arquivo_saida, "wb") as arquivo:
            arquivo.write(saida[0])

    pla_obj = pla_obj_factory_old(saida[0])

    tempo_final = time.time() - tempo_inicial
    retorno = {"nome_arquivo":      arquivo_saida,
               "arquivo_gerado":    gera_arquivo,
               "tempo":             tempo_final,
               "saida_espresso":    saida[0],
               "pla_obj":           pla_obj,
               "erro":              saida[1]}
    return retorno
