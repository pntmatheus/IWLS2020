import time

import argparse
import sys
import os

from Functions.scriptABCFunctions import circuit_abc_map

def read_files(genlib_file, circuit_file, generated_folder):

    with open(genlib_file, 'r') as reader:
        genlibs = reader.readlines()

    with open(circuit_file, 'r') as reader:
        circuits = reader.readlines()

    if generated_folder is None:
        output_folder = None
    else:
        if generated_folder[-1] is "/":
            output_folder = generated_folder
        else:
            output_folder = generated_folder + "/"
            print(output_folder)

    for g in genlibs:
        for c in circuits:
            g_name = g.split("/")[-1].split(".")[0]
            c_name = c.split("/")[-1].split(".")[0]
            mapped_name = "%s_%s.v" % (c_name, g_name)

            if output_folder is None:
                circuit_abc_map(g, c, mapped_name)
            else:
                name = output_folder + mapped_name
                circuit_abc_map(g, c, name)



def main(argv):

    parser = argparse.ArgumentParser(description="ABC wrapper for mapping circuits",
                                     epilog="Colocar um exemplo de uso...")
    parser.add_argument("-g", "--genlib", help="Genlib filepath", dest="genlib_list", required=True, metavar="<file.pla>")
    parser.add_argument("-c", "--circuit", help="Circuit description filepath", dest="circuit_list", metavar="</my/path/name.aig>")
    parser.add_argument("-o", "--output_directory",
                        help="(Optional) local to generated files", dest="generated_folder")


    result = parser.parse_args()

    genlib_list = result.genlib_list
    circuit_list = result.circuit_list
    generated_folder = result.generated_folder

    read_files(genlib_list, circuit_list, generated_folder)

if __name__ == '__main__':
    tempo_inicial = time.time()
    main(sys.argv[1:])
    #lib_path = "tools/files/genlibs/full_no_costV2.genlib"
    #circ_desc_path = "tools/files/benchmarks/ISCAS85/blif/c432.blif"
    #circ_mapped = "tools/files/mappeds/c432_full_no_caostV2.v"

    #circuit_abc_map(lib_path, circ_desc_path, circ_mapped)



    print("Tempo final %f " % (time.time() - tempo_inicial))