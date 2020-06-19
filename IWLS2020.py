import argparse
import sys
import os

from Functions.pla import pla_obj_factory
from Functions.wekaWrapper import gera_melhor_aig


def check_tmp_directory():
    def check_dir_exists(path):
        if not os.path.exists(path):
            os.mkdir(d)

    dirs = ["tmp_iwls2020",
            "tmp_iwls2020/AAG",
            "tmp_iwls2020/PART",
            "tmp_iwls2020/PART_AIG",
            "tmp_iwls2020/J48",
            "tmp_iwls2020/J48_AIG",
            "tmp_iwls2020/J48_PLA",
            "tmp_iwls2020/NEURAL_PLAs",
            "tmp_iwls2020/ARFF",
            "persist_iwls2020"]

    for d in dirs:
        check_dir_exists(d)


def main(argv):

    parser = argparse.ArgumentParser(description="IWLS 2020 Contest AIG function predictor",
                                     epilog="Usage example: python3.8 IWLS2020.py -i ex00.pla -o my_ex00.aig")
    parser.add_argument("-i", help="Input PLA file path", dest="input_file", required=True, metavar="<file.pla>")
    parser.add_argument("-o", help="Output AIG name", dest="output_file", metavar="</my/path/name.aig>")
    parser.add_argument("-v", "--verbose",
                        help='Verbose mode, output all options tested by tool in the format '
                             '"<accuracy;;AIG_ANDs;;AIG_Depth;;runtime;;tmp_file>" (Default mode) '
                             'or "<accuracy;;AIG_ANDs;;mltest_file1;;...;;mltest_fileN;;AIG_Depth;;runtime;;tmp_file'
                             '>" (Mltest mode)',
                        action="store_true")
    parser.add_argument("--mltest", help="Especial mode to eval generated AIG against PLA files",
                        dest="mltests", nargs='+', metavar=("pla_file1", "pla_fileN"))
    parser.add_argument("--supress", help="Supress output messages", action="store_true")
    parser.add_argument("--persist", help="Persist all option files in persist_iwls folder", action="store_true")
    parser.add_argument("--dont-clean-tmp", help="Option for multiprocessing goal", action="store_true")

    result = parser.parse_args()

    in_file = result.input_file
    out_file = result.output_file
    v_mode = result.verbose
    mltest_files = result.mltests
    dont_clean = result.dont_clean_tmp

    pla_obj = pla_obj_factory(in_file)

    gera_melhor_aig(pla_obj,
                    supress=result.supress,
                    mltest_list=mltest_files,
                    out_filename=out_file,
                    persist=result.persist,
                    verbose=v_mode,
                    tmp_clean=dont_clean)


if __name__ == "__main__":
    check_tmp_directory()
    main(sys.argv[1:])
