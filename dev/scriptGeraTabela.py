from scriptEspressoFunctions import *
from scriptABCFunctions import *
from Functions.pla import pla_obj_factory


def retorna_linha_tabela(caminho, pla_original, nome_saida, timeout):
    saida = list()

    caminho_pla = ("%s/%s" % (caminho,pla_original))
    nome_ex = pla_original.split(".")[0]
    nome_aig_abc_direto = pla_original.replace(".pla", ".ABC-DIRETO.aig")
    nome_espresso_pla = pla_original.replace(".pla", ".ESPRESSO.pla")

    descricao_pla = le_pla(caminho_pla)

    in_out = "%d / %d" % (descricao_pla["qt_ins"], descricao_pla["qt_outs"])
    qt_on_sets = len(descricao_pla["termos_on_set"])
    qt_off_sets = len(descricao_pla["termos_off_set"])

    espresso_pla = gera_espresso_pla(caminho_pla, nome_espresso_pla, "", True, timeout)

    # Gera o AIG sem passar pelo ESPRESSO
    abc_gera_aig = gera_abc_aig(caminho_pla,
                                ["resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2"],
                                nome_aig_abc_direto)
    tempo_abc_original = abc_gera_aig["tempo"]

    mltest_train_aig_abc_direto = abc_mltest_aig(nome_aig_abc_direto, "%s/%s.train.pla" % (caminho, nome_ex))
    mltest_valid_aig_abc_direto = abc_mltest_aig(nome_aig_abc_direto, "%s/%s.valid.pla" % (caminho, nome_ex))



    print("################   %s  ###############" % pla_original)

    print(in_out)
    saida.append(in_out)
    print("IN_OUT_Direto %d / %d" % (qt_on_sets, qt_off_sets))
    saida.append(str(qt_on_sets))
    saida.append(str(qt_off_sets))
    print("ANDs_Direto %d" % mltest_valid_aig_abc_direto["ands"])
    saida.append(mltest_valid_aig_abc_direto["ands"])
    print("VALID_Direto %.3f" % mltest_valid_aig_abc_direto["per_acerto"])
    saida.append("%.3f" % mltest_valid_aig_abc_direto["per_acerto"])
    print("TRAIN_Direto %.3f" % mltest_train_aig_abc_direto["per_acerto"])
    saida.append("%.3f" % mltest_train_aig_abc_direto["per_acerto"])
    print("TEMPO_Direto ===> %f " % tempo_abc_original)
    saida.append(str(tempo_abc_original))

    if espresso_pla is None:
        for i in range(5):
            saida.append(" ")

    else:
        nome_aig_espresso = nome_espresso_pla.replace(".pla", ".aig")
        abc_gera_aig_espresso = gera_abc_aig(nome_espresso_pla,
                                ["resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2"],
                                nome_aig_espresso)

        mltest_train_aig_espresso = abc_mltest_aig(nome_aig_espresso, "%s/%s.train.pla" % (caminho, nome_ex))
        mltest_valid_aig_espresso = abc_mltest_aig(nome_aig_espresso, "%s/%s.valid.pla" % (caminho, nome_ex))

        print("ESPRESSO ON_SETS =====> %d" % len(espresso_pla["pla_obj"]["termos_on_set"]))
        saida.append(str(len(espresso_pla["pla_obj"]["termos_on_set"])))
        print("ESPRESSO AND_Direto %d" % mltest_valid_aig_espresso["ands"])
        saida.append(str(mltest_valid_aig_espresso["ands"]))
        print("ESPRESSO VALID %.3f" % mltest_valid_aig_espresso["per_acerto"])
        saida.append("%.3f" % mltest_valid_aig_espresso["per_acerto"])
        print("ESPRESSO TRAIN %.3f" % mltest_train_aig_espresso["per_acerto"])
        saida.append("%.3f" % mltest_train_aig_espresso["per_acerto"])
        print("ESPRESSO TEMPO %d" % espresso_pla["tempo"])
        saida.append(str(espresso_pla["tempo"]))

    retorno = "".join(["%s," % i for i in saida])
    print(retorno)
    return retorno

def gera_contagem_variaveis():
    with open("espressos.txt", "r") as arquivo:
        for teste in arquivo.readlines():
            nome_pla = teste.replace(".pla", ".ESPRESSO.pla")
            nome_pla = nome_pla.replace("\n", "")
            pasta = "Resultados/PLAs-AIGs-TABELAO"
            pla_obj = pla_obj_factory(nome_pla, "%s/%s" % (pasta, nome_pla))
            linha = "%d;;%d;;%d" % (pla_obj.get_total_pla_0(), pla_obj.get_total_pla_1(), pla_obj.get_total_pla_dcare())
            print(linha)
            with open("testeESPRESSO0E1s.txt", "a+") as arquivo:
                arquivo.write(linha + "\n")


def conta_repetidos():
    for i in range(100):
        for versao in ["train", "valid", "total"]:
            nome_pla = "ex%02d.%s.pla" % (i, versao)
            pasta = "PLA_files"
            pla_obj = pla_obj_factory(nome_pla, "%s/%s" % (pasta, nome_pla))
            qt_repetidos = len(pla_obj.get_termos()) - len(pla_obj.get_termos_unique())
            linha = "%d;;%d;;%d" % (qt_repetidos,pla_obj.get_total_pla_0(),pla_obj.get_total_pla_1())
            print("%s --> %s" % (nome_pla,linha))
            with open("originais0E1s.txt", "a+") as arquivo:
                arquivo.write(linha + "\n")


if __name__ == '__main__':
    tempo_inicial = time.time()
    plas = ["7-INPUT-TABLE.pla", "7-INPUT-TRAIN.pla", "7-INPUT-TRAIN64.pla"]
    for pla in plas:
        gera_espresso_pla(pla, pla.replace(".pla", "-ESPRESSO.pla"), gera_arquivo=True)

    plas = ["7-INPUT-TABLE.pla",
            "7-INPUT-TRAIN.pla",
            "7-INPUT-TRAIN64.pla",
            "7-INPUT-TABLE-ESPRESSO.pla",
            "7-INPUT-TRAIN-ESPRESSO.pla",
            "7-INPUT-TRAIN64-ESPRESSO.pla",
            "7-INPUT-TRAIN-BUTZEN-MINIMIZATION.pla"]

    for pla in plas:

        pla_obj = pla_obj_factory(pla)
        gera_abc_aig(pla_obj.nome, pla_obj.nome.replace(".pla", ".aig"))

        mltest_table = abc_mltest_aig(pla_obj.nome.replace(".pla", ".aig"), "7-INPUT-TABLE.pla")
        mltest_train = abc_mltest_aig(pla_obj.nome.replace(".pla", ".aig"), "7-INPUT-TRAIN64.pla")

        linha = "%d;;%d;;%d;;%d;;%d;;%s;;%s;;%s" % (len(pla_obj.get_offsets()),
                                        len(pla_obj.get_onsets()),
                                        pla_obj.get_total_pla_0(),
                                        pla_obj.get_total_pla_1(),
                                        pla_obj.get_total_pla_dcare(),
                                        mltest_table["ands"],
                                        mltest_table["per_acerto"],
                                        mltest_train["per_acerto"])
        print(linha)
    print("Tempo final %f " % (time.time() - tempo_inicial))



