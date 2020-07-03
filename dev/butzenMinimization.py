import time
import bisect


def pega_termos_literais(literais, termos):
    for t in termos:
        print(len(t.get_input()))


def retorna_contagem(lista_termos, qt_inputs, literais=None):
    print("QT-INPUTS ===> %s " % qt_inputs)
    if not literais:
        retorno = list()

        tabela_quantitativo = list()
        # inicializar a tabela de quantitativos
        # TODO deixar pronto para qualquer número de saídas

        inicial = time.time()

        # Pq 2 x inputs??
        # Pq considero A e A!
        for i in range(2 * qt_inputs):
            # [<qt_0>, <qt_1>]
            tabela_quantitativo.append([0, 0])

        for termo in lista_termos:
            if termo.output == "0":
                for i, char in enumerate(termo.get_input()):
                    posicao = 2 * i + int(char)
                    tabela_quantitativo[posicao][0] += 1
            else:
                for i, char in enumerate(termo.get_input()):
                    posicao = 2 * i + int(char)
                    tabela_quantitativo[posicao][1] += 1

        dict_quantitativo = {}

        inicial = time.time()
        for i, variavel in enumerate(tabela_quantitativo):
            # Aqui coloco no dicionario [<qt_1>, <diff_qt_1_E_qt_0>]
            dict_quantitativo[i] = [variavel[1], (variavel[1] - variavel[0])]

        maior_var = max(dict_quantitativo.items(), key=lambda x: x[1])
        bisect.insort(retorno, maior_var[0])

        dict_quantitativo.pop(maior_var[0])
        maior_var = max(dict_quantitativo.items(), key=lambda x: x[1])
        bisect.insort(retorno, maior_var[0])

        return retorno
    else:
        print("PAPAI")
        print(literais)


def pega_mais_leva_pra_um(qt_inputs, termos, ja_contadas=False):

    primeiros = retorna_contagem(termos, qt_inputs)
    print(primeiros)

    pega_termos_literais(primeiros, termos)

    time.sleep(15)
    retorno = list()

    tabela_quantitativo = list()
    # inicializar a tabela de quantitativos
    # TODO deixar pronto para qualquer número de saídas

    inicial = time.time()

    # Pq 2 x inputs??
    # Pq considero A e A!
    for i in range(2 * qt_inputs):
        # [<qt_0>, <qt_1>]
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

    for tab in tabela_quantitativo:
        print(tab)

    dict_quantitativo = {}

    inicial = time.time()
    for i, variavel in enumerate(tabela_quantitativo):
        # Aqui coloco no dicionario [<qt_1>, <diff_qt_1_E_qt_0>]
        dict_quantitativo[i] = [variavel[1], (variavel[1] - variavel[0])]

    maior_var = max(dict_quantitativo.items(), key=lambda x: x[1])

    bisect.insort(retorno, maior_var[0])

    dict_quantitativo.pop(maior_var[0])

    maior_var = max(dict_quantitativo.items(), key=lambda x: x[1])

    bisect.insort(retorno, maior_var[0])
    print("Tempo CONTA os maiores ===> %f" % (time.time() - inicial))

    return retorno
