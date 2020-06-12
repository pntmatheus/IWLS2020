import time
import bisect

from pla import Pla, pla_obj_factory


def pega_mais_leva_pra_um(qt_inputs, termos, ja_contadas=False):

    retorno = list()

    tabela_quantitativo = list()
    # inicializar a tabela de quantitativos
    # TODO deixar pronto para qualquer número de saídas
    inicial = time.time()
    for i in range(2 * qt_inputs):
        tabela_quantitativo.append([0, 0])
    print("Tempo FOR criacao tabela_quantitativo ===> %f" % (time.time() - inicial))

    inicial = time.time()
    for termo in termos:
        if termo.output == "0":
            for i, char in enumerate(termo.get_input()):
                posicao = 2 * i + int(char)
                tabela_quantitativo[posicao][0] += 1
        else:
            for i, char in enumerate(termo.get_input()):
                posicao = 2 * i + int(char)
                tabela_quantitativo[posicao][1] += 1
    print("Tempo FOR que conta as variaveis ===> %f" % (time.time() - inicial))


    dict_quantitativo = {}

    inicial = time.time()
    for i, variavel in enumerate(tabela_quantitativo):
        dict_quantitativo[i] = [variavel[1], (variavel[1] - variavel[0])]

    maior_var = max(dict_quantitativo.items(), key=lambda x: x[1])

    bisect.insort(retorno, maior_var[0])

    dict_quantitativo.pop(maior_var[0])

    maior_var = max(dict_quantitativo.items(), key=lambda x: x[1])

    bisect.insort(retorno, maior_var[0])
    print("Tempo CONTA os maiores ===> %f" % (time.time() - inicial))

    return retorno


if __name__ == '__main__':
    tempo_inicial = time.time()

    pla_obj = pla_obj_factory("/media/sf_PastaUbuntuServer/ex90.total.pla")
    # pla_obj = pla_obj_factory("ex41.total.pla")

    print(pega_mais_leva_pra_um(pla_obj.get_qt_inputs(), pla_obj.get_termos()))

    print("Tempo final %f " % (time.time() - tempo_inicial))