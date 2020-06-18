from scriptCommonFunctions import *
from scriptEspressoFunctions import *
from Functions.scriptABCFunctions import *

if __name__ == '__main__':
    tempo_inicial = time.time()
    # teste = gera_espresso_pla("ex41.total.pla", "testandoPycharm67dasdasd8.pla", ["-eonset", "-efast"], True)
    # pla_obj = teste["pla_obj"]
    # pla_obj = le_pla("ex41.total.pla")
    # cria_arquivo_pla(pla_obj, "testandoOCriaPla.pla")

    def gera_resultado(nome_pla, versao_pla):

        nome_pla = nome_pla
        versao_pla = versao_pla

        nome_arquivo = "%s.%s.pla" % (nome_pla, versao_pla)

        pla = le_pla(nome_arquivo)

        print("inputs ===> %d" % pla["qt_ins"])
        print("outputs ===> %d" % pla["qt_outs"])
        print("total termos ===> %d" % pla["qt_termos"])
        print("total on-sets ===> %d" % len(pla["termos_on_set"]))
        print("total off-sets ===> %d" % len(pla["termos_off_set"]))
        print("VERSOES ABC")

        print("\n\n\n##################################################")
        print("### %s ####" % nome_arquivo)
        print("###########")
        print("")


        print("-----------------------------------------------")
        print("ABC %s RAW" % nome_arquivo)
        abc_gera_aig = gera_abc_aig(nome_arquivo, "", nome_arquivo.replace(".pla", "RAW.aig"))
        dados_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "RAW.aig"), "%s.train.pla" % nome_pla)
        valid_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "RAW.aig"), "%s.valid.pla" % nome_pla)
        total_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "RAW.aig"), "%s.total.pla" % nome_pla)
        print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"],dados_aig["levels"],dados_aig["per_acerto"],valid_aig["per_acerto"],total_aig["per_acerto"]))

        abc_gera_aig = gera_abc_aig(nome_arquivo, ["resyn2"], nome_arquivo.replace(".pla", "1x-RESYN2.aig"))
        dados_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "1x-RESYN2.aig"), "%s.train.pla" % nome_pla)
        valid_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "1x-RESYN2.aig"), "%s.valid.pla" % nome_pla)
        total_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "1x-RESYN2.aig"), "%s.total.pla" % nome_pla)
        print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"], dados_aig["levels"], dados_aig["per_acerto"], valid_aig["per_acerto"],total_aig["per_acerto"]))


        abc_gera_aig = gera_abc_aig(nome_arquivo, ["resyn2", "resyn2"],
                                    nome_arquivo.replace(".pla", "2x-RESYN2.aig"))
        dados_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "2x-RESYN2.aig"), "%s.train.pla" % nome_pla)
        valid_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "2x-RESYN2.aig"), "%s.valid.pla" % nome_pla)
        total_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "2x-RESYN2.aig"), "%s.total.pla" % nome_pla)
        print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"],dados_aig["levels"],dados_aig["per_acerto"],valid_aig["per_acerto"],total_aig["per_acerto"]))


        abc_gera_aig = gera_abc_aig(nome_arquivo, ["resyn2", "resyn2", "resyn2", "resyn2"],
                                    nome_arquivo.replace(".pla", "4x-RESYN2.aig"))
        dados_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "4x-RESYN2.aig"), "%s.train.pla" % nome_pla)

        valid_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "4x-RESYN2.aig"), "%s.valid.pla" % nome_pla)

        total_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "4x-RESYN2.aig"), "%s.total.pla" % nome_pla)
        print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"],dados_aig["levels"],dados_aig["per_acerto"],valid_aig["per_acerto"],total_aig["per_acerto"]))


        abc_gera_aig = gera_abc_aig(nome_arquivo, ["resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2"],
                                    nome_arquivo.replace(".pla", "8x-RESYN2.aig"))
        dados_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "8x-RESYN2.aig"), "%s.train.pla" % nome_pla)
        valid_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "8x-RESYN2.aig"), "%s.valid.pla" % nome_pla)
        total_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "8x-RESYN2.aig"), "%s.total.pla" % nome_pla)
        print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"],dados_aig["levels"],dados_aig["per_acerto"],valid_aig["per_acerto"],total_aig["per_acerto"]))


        abc_gera_aig = gera_abc_aig(nome_arquivo,
                                    ["resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2",
                                                 "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2"],
                                    nome_arquivo.replace(".pla", "16x-RESYN2.aig"))
        dados_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "16x-RESYN2.aig"), "%s.train.pla" % nome_pla)
        valid_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "16x-RESYN2.aig"), "%s.valid.pla" % nome_pla)
        total_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "16x-RESYN2.aig"), "%s.total.pla" % nome_pla)
        print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"],dados_aig["levels"],dados_aig["per_acerto"],valid_aig["per_acerto"],total_aig["per_acerto"]))


        abc_gera_aig = gera_abc_aig(nome_arquivo, ["collapse", "strash"], nome_arquivo.replace(".pla", "COLLAPSE.aig"))
        dados_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "COLLAPSE.aig"), "%s.train.pla" % nome_pla)
        valid_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "COLLAPSE.aig"), "%s.valid.pla" % nome_pla)
        total_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "COLLAPSE.aig"), "%s.total.pla" % nome_pla)
        print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"], dados_aig["levels"], dados_aig["per_acerto"], valid_aig["per_acerto"],total_aig["per_acerto"]))


        abc_gera_aig = gera_abc_aig(nome_arquivo, ["collapse", "strash", "resyn2", "resyn2", "resyn2", "resyn2"], nome_arquivo.replace(".pla", "COLLAPSE-4x-RESYN2.aig"))
        dados_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "COLLAPSE-4x-RESYN2.aig"), "%s.train.pla" % nome_pla)
        valid_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "COLLAPSE-4x-RESYN2.aig"), "%s.valid.pla" % nome_pla)
        total_aig = abc_mltest_aig(nome_arquivo.replace(".pla", "COLLAPSE-4x-RESYN2.aig"), "%s.total.pla" % nome_pla)
        print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"], dados_aig["levels"], dados_aig["per_acerto"], valid_aig["per_acerto"],total_aig["per_acerto"]))

        print("##########################################\n\n\n\n\n\n")


    gera_resultado("ex30", "train")
    gera_resultado("ex30", "valid")
    gera_resultado("ex30", "total")
    gera_resultado("ex43", "train")
    gera_resultado("ex43", "valid")
    gera_resultado("ex43", "total")
    gera_resultado("ex64", "train")
    gera_resultado("ex64", "valid")
    gera_resultado("ex64", "total")
    '''
            espresso_cmd = ["",
                            "-efast",
                            "-eonset",
                            "-estrong",
                            "-Dd1merge"]

            for cmd in espresso_cmd:
                espresso_saida = "%s-ESPRESSO%s.%s.pla" % (nome_pla, cmd.upper(), versao_pla)
                espresso = gera_espresso_pla(nome_arquivo, espresso_saida, [cmd], True)

                print("\n\n\n##################################################")
                print("### %s ####" % espresso["nome_arquivo"])
                print("###########")
                print("")

                print("TERMOS E TEMPO")
                print("%d,%.3f" % (espresso["pla_obj"]["qt_termos"], espresso["tempo"]))

                print("-----------------------------------------------")
                print("ABC %s RAW" % espresso_saida)
                abc_gera_aig = gera_abc_aig_alternativo(espresso_saida, "", espresso_saida.replace(".pla", "RAW.aig"))
                dados_aig = abc_mltest_aig(espresso_saida.replace(".pla", "RAW.aig"), "%s.train.pla" % nome_pla)
                valid_aig = abc_mltest_aig(espresso_saida.replace(".pla", "RAW.aig"), "%s.valid.pla" % nome_pla)
                total_aig = abc_mltest_aig(espresso_saida.replace(".pla", "RAW.aig"), "%s.total.pla" % nome_pla)
                print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"],dados_aig["levels"],dados_aig["per_acerto"],valid_aig["per_acerto"],total_aig["per_acerto"]))

                abc_gera_aig = gera_abc_aig_alternativo(espresso_saida, ["resyn2"], espresso_saida.replace(".pla", "1x-RESYN2.aig"))
                dados_aig = abc_mltest_aig(espresso_saida.replace(".pla", "1x-RESYN2.aig"), "%s.train.pla" % nome_pla)
                valid_aig = abc_mltest_aig(espresso_saida.replace(".pla", "1x-RESYN2.aig"), "%s.valid.pla" % nome_pla)
                total_aig = abc_mltest_aig(espresso_saida.replace(".pla", "1x-RESYN2.aig"), "%s.total.pla" % nome_pla)
                print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"], dados_aig["levels"], dados_aig["per_acerto"], valid_aig["per_acerto"],total_aig["per_acerto"]))


                abc_gera_aig = gera_abc_aig_alternativo(espresso_saida, ["resyn2", "resyn2"],
                                                        espresso_saida.replace(".pla", "2x-RESYN2.aig"))
                dados_aig = abc_mltest_aig(espresso_saida.replace(".pla", "2x-RESYN2.aig"), "%s.train.pla" % nome_pla)
                valid_aig = abc_mltest_aig(espresso_saida.replace(".pla", "2x-RESYN2.aig"), "%s.valid.pla" % nome_pla)
                total_aig = abc_mltest_aig(espresso_saida.replace(".pla", "2x-RESYN2.aig"), "%s.total.pla" % nome_pla)
                print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"],dados_aig["levels"],dados_aig["per_acerto"],valid_aig["per_acerto"],total_aig["per_acerto"]))


                abc_gera_aig = gera_abc_aig_alternativo(espresso_saida, ["resyn2", "resyn2", "resyn2", "resyn2"],
                                                        espresso_saida.replace(".pla", "4x-RESYN2.aig"))
                dados_aig = abc_mltest_aig(espresso_saida.replace(".pla", "4x-RESYN2.aig"), "%s.train.pla" % nome_pla)

                valid_aig = abc_mltest_aig(espresso_saida.replace(".pla", "4x-RESYN2.aig"), "%s.valid.pla" % nome_pla)

                total_aig = abc_mltest_aig(espresso_saida.replace(".pla", "4x-RESYN2.aig"), "%s.total.pla" % nome_pla)
                print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"],dados_aig["levels"],dados_aig["per_acerto"],valid_aig["per_acerto"],total_aig["per_acerto"]))


                abc_gera_aig = gera_abc_aig_alternativo(espresso_saida, ["resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2"],
                                                        espresso_saida.replace(".pla", "8x-RESYN2.aig"))
                dados_aig = abc_mltest_aig(espresso_saida.replace(".pla", "8x-RESYN2.aig"), "%s.train.pla" % nome_pla)
                valid_aig = abc_mltest_aig(espresso_saida.replace(".pla", "8x-RESYN2.aig"), "%s.valid.pla" % nome_pla)
                total_aig = abc_mltest_aig(espresso_saida.replace(".pla", "8x-RESYN2.aig"), "%s.total.pla" % nome_pla)
                print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"],dados_aig["levels"],dados_aig["per_acerto"],valid_aig["per_acerto"],total_aig["per_acerto"]))


                abc_gera_aig = gera_abc_aig_alternativo(espresso_saida,
                                                        ["resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2",
                                                         "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2", "resyn2"],
                                                        espresso_saida.replace(".pla", "16x-RESYN2.aig"))
                dados_aig = abc_mltest_aig(espresso_saida.replace(".pla", "16x-RESYN2.aig"), "%s.train.pla" % nome_pla)
                valid_aig = abc_mltest_aig(espresso_saida.replace(".pla", "16x-RESYN2.aig"), "%s.valid.pla" % nome_pla)
                total_aig = abc_mltest_aig(espresso_saida.replace(".pla", "16x-RESYN2.aig"), "%s.total.pla" % nome_pla)
                print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"],dados_aig["levels"],dados_aig["per_acerto"],valid_aig["per_acerto"],total_aig["per_acerto"]))


                abc_gera_aig = gera_abc_aig_alternativo(espresso_saida, ["collapse", "strash"], espresso_saida.replace(".pla", "COLLAPSE.aig"))
                dados_aig = abc_mltest_aig(espresso_saida.replace(".pla", "COLLAPSE.aig"), "%s.train.pla" % nome_pla)
                valid_aig = abc_mltest_aig(espresso_saida.replace(".pla", "COLLAPSE.aig"), "%s.valid.pla" % nome_pla)
                total_aig = abc_mltest_aig(espresso_saida.replace(".pla", "COLLAPSE.aig"), "%s.total.pla" % nome_pla)
                print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"], dados_aig["levels"], dados_aig["per_acerto"], valid_aig["per_acerto"],total_aig["per_acerto"]))


                abc_gera_aig = gera_abc_aig_alternativo(espresso_saida, ["collapse", "strash", "resyn2", "resyn2", "resyn2", "resyn2" ], espresso_saida.replace(".pla", "COLLAPSE-4x-RESYN2.aig"))
                dados_aig = abc_mltest_aig(espresso_saida.replace(".pla", "COLLAPSE-4x-RESYN2.aig"), "%s.train.pla" % nome_pla)
                valid_aig = abc_mltest_aig(espresso_saida.replace(".pla", "COLLAPSE-4x-RESYN2.aig"), "%s.valid.pla" % nome_pla)
                total_aig = abc_mltest_aig(espresso_saida.replace(".pla", "COLLAPSE-4x-RESYN2.aig"), "%s.total.pla" % nome_pla)
                print("%.3f,%d,%d,%.3f,%.3f,%.3f" % (abc_gera_aig["tempo"], dados_aig["ands"], dados_aig["levels"], dados_aig["per_acerto"], valid_aig["per_acerto"],total_aig["per_acerto"]))

                print("##########################################\n\n\n\n\n\n")
    '''
    print("Tempo final %f " % (time.time() - tempo_inicial))
