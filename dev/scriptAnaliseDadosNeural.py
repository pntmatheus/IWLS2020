import time
import csv

def tota_analise():
    current_dict = dict()
    rows = list()
    current_pla = "primeiro"
    with open('IWLS2020--AIGs.csv') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for row in csv_reader:
            if row[0] != "":
                current_dict[current_pla] = rows.copy()
                rows = list()
                current_pla = row[0].replace(" ", "")
            else:
                rows.append(row)
        # Pegar o último
        current_dict[current_pla] = rows.copy()

    # inicializacao das flags
    exact_100 = 0
    f_95_99 = 0
    f_90_95 = 0
    smaller_90 = 0
    melhores_options = dict()
    for key, value in current_dict.items():
        if "TOTAL" in key:
            train_mltest = 0.0
            best_train = ""
            ands = 0
            for t in value:
                flag_mltest = (float(t[6]) + float(t[5])) / 2
                if flag_mltest > train_mltest:
                    if 5000 > int(t[4]):
                        best_train = t[1]
                        train_mltest = flag_mltest
                        ands = int(t[4])
            if train_mltest == 100.0:
                exact_100 += 1
            else:
                if 95.0 < train_mltest < 100:
                    f_95_99 += 1
                else:
                    if 90.0 < train_mltest < 95.0:
                        f_90_95 += 1
                    else:
                        smaller_90 += 1
                        print(key)

            if best_train in melhores_options:
                melhores_options[best_train] += 1
            else:
                melhores_options[best_train] = 1
            print("No circuito %s o %s foi a melhor opcao e deu %f com %d ANDS" % (key, best_train, train_mltest, ands))
    print("Exato 100%%: %d" % exact_100)
    print("Entre 95%% e 99%%: %d" % f_95_99)
    print("Entre 90%% e 95%%: %d" % f_90_95)
    print("Menor que 90%%: %d" % smaller_90)

    order_melhores = sorted(melhores_options, key=melhores_options.get, reverse=True)

    for i in order_melhores:
        print("%s;;%d" % (i, melhores_options[i]))


def valid_train_analise():
    current_dict = dict()
    rows = list()
    current_pla = "primeiro"
    with open('IWLS2020--AIGs.csv') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for row in csv_reader:
            if row[0] != "":
                current_dict[current_pla] = rows.copy()
                rows = list()
                current_pla = row[0].replace(" ", "")
            else:
                rows.append(row)
        # Pegar o último
        current_dict[current_pla] = rows.copy()

    # inicializacao das flags
    greater_90 = 0
    f_80_90 = 0
    f_70_80 = 0
    f_60_70 = 0
    f_50_60 = 0
    smaller_50 = 0
    melhores_options = dict()
    for key, value in current_dict.items():
        if "VALID" in key:
            train_mltest = 0.0
            best_train = ""
            ands = 0
            for t in value:
                if float(t[6]) > train_mltest:
                    if float(t[5]) > 60:
                        if 5000 > int(t[4]):
                            best_train = t[1]
                            train_mltest = float(t[6])
                            ands = int(t[4])
            if train_mltest > 90.0:
                greater_90 += 1
            else:
                if 80.0 < train_mltest < 90.0:
                    f_80_90 += 1
                else:
                    if 70.0 < train_mltest < 80.0:
                        f_70_80 += 1
                    else:
                        if 60.0 < train_mltest < 70.0:
                            f_60_70 += 1
                        else:
                            if 50.0 < train_mltest < 60.0:
                                f_50_60 += 1
                            else:
                                smaller_50 += 1
            if best_train in melhores_options:
                melhores_options[best_train] += 1
            else:
                melhores_options[best_train] = 1
            print("No circuito %s o %s foi a melhor opcao e deu %f com %d ANDS" % (key, best_train, train_mltest, ands))
    print("Maior que 90%%: %d" % greater_90)
    print("Entre 80%% e 90%%: %d" % f_80_90)
    print("Entre 70%% e 80%%: %d" % f_70_80)
    print("Entre 60%% e 70%%: %d" % f_60_70)
    print("Entre 50%% e 60%%: %d" % f_50_60)
    print("Menor que 50%%: %d" % smaller_50)

    order_melhores = sorted(melhores_options, key=melhores_options.get, reverse=True)

    for i in order_melhores:
        print("%s;;%d" % (i, melhores_options[i]))


def tem_valid_na_lista(lista_atual, mltest_train, mltest_valid, option):
    if len(lista_atual) != 0:
        for opt in lista_atual:
            if opt[5] == mltest_train:
                if opt[6] != mltest_valid:
                    return False
                else:
                    return True
        return False
    else:
        return False



def novo_valid_train_analise():
    current_dict = dict()
    rows = list()
    current_pla = "primeiro"
    with open('IWLS2020--AIGs.csv') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for row in csv_reader:
            if row[0] != "":
                current_dict[current_pla] = rows.copy()
                rows = list()
                current_pla = row[0].replace(" ", "")
            else:
                rows.append(row)
        # Pegar o último
        current_dict[current_pla] = rows.copy()

    # inicializacao das flags
    greater_90 = list()
    f_80_90 = list()
    f_70_80 = list()
    f_60_70 = list()
    f_50_60 = list()
    smaller_50 = list()
    melhores_options = dict()

    for key, value in current_dict.items():
        if "TRAIN" in key:
            result = list()

            train_mltest = 0.0
            best_train = ""
            ands = 0

            for t in value:
                if not tem_valid_na_lista(result, t[5], t[6], t[1]):
                    result.append(t)

            print("TOTAL: %d" % len(result))
            for t in result:
                if float(t[5]) > train_mltest:
                    if float(t[6]) > 80:
                        if 5000 > int(t[4]):
                            best_train = t[1]
                            train_mltest = float(t[5])
                            ands = int(t[4])
            if train_mltest > 90.0:
                greater_90.append(key)
            else:
                if 80.0 < train_mltest < 90.0:
                    f_80_90.append(key)
                else:
                    if 70.0 < train_mltest < 80.0:
                        f_70_80.append(key)
                    else:
                        if 60.0 < train_mltest < 70.0:
                            f_60_70.append(key)
                        else:
                            if 50.0 < train_mltest < 60.0:
                                f_50_60.append(key)
                            else:
                                smaller_50.append(key)
            if best_train in melhores_options:
                melhores_options[best_train].append(key)
            else:
                melhores_options[best_train] = list()
                melhores_options[best_train].append(key)
            print("No circuito %s o %s foi a melhor opcao e deu %f com %d ANDS" % (key, best_train, train_mltest, ands))
    print("Maior que 90%%: %d" % len(greater_90))
    print("Entre 80%% e 90%%: %d" % len(f_80_90))
    print("Entre 70%% e 80%%: %d" % len(f_70_80))
    print("Entre 60%% e 70%%: %d" % len(f_60_70))
    print("Entre 50%% e 60%%: %d" % len(f_50_60))
    print("Menor que 50%%: %d" % len(smaller_50))

    print(melhores_options)

#    order_melhores = sorted(melhores_options, key=melhores_options.get, reverse=True)
    order_melhores = sorted(melhores_options, key=lambda k: len(melhores_options[k]), reverse=True)

    for i in order_melhores:
        print("%s;;%d" % (i, len(melhores_options[i])))

    for i in order_melhores:
        for b in melhores_options[i]:
            print("%s;;%s" % (i, b))

    #print("########## GREATER 90 ##########")
    for f in greater_90:
        print("%s;;%s" % ("greater90",f))

    #print("########## 80 --- 90 ##########")
    for f in f_80_90:
        print("%s;;%s" % ("80---90", f))

    #print("########## 70 --- 80 ##########")
    for f in f_70_80:
        print("%s;;%s" % ("70---80", f))

    #print("########## 60 --- 70 ##########")
    for f in f_60_70:
        print("%s;;%s" % ("60---70", f))

    #print("########## 50 --- 60 ##########")
    for f in f_50_60:
        print("%s;;%s" % ("50---60", f))

    #print("########## SMALLER 50 ##########")
    for f in smaller_50:
        print("%s;;%s" % ("smaller50", f))

if __name__ == '__main__':
    tempo_inicial = time.time()

    novo_valid_train_analise()

    print("Tempo final %f " % (time.time() - tempo_inicial))