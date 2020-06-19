# IWLS2020
The developed software reads a PLA file as input and creates an AIG file as output.
We used Python as the principal programming language, but we also used JAVA and C languages as part of the solution.
To try to get the unknown boolean function, the proposed solution uses J48 and PART Artificial Intelligence classifiers.
We created a single file, combining both train and valid files, to use as input. The algorithm first transforms the PLA file in an ARFF to be handled by the WEKA tool. We used the WEKA tool to run five different configurations to the J48 classifier and also five configurations to the PART classifier, varying the confidence factor (“-C” option in WEKA).  It is applied cross-validation to decide what is the best option. We chose the best classifier and confidence option, and we used the minimum number of objects (“-M”) to test 6 more variants. Finally, we chose the best option. We used the ABC tool to check the size of the generated AIGs to match the contest requirements. 
The J48 classifier creates a decision tree that the developed software converts in a PLA file. In the sequence, the ABC tool transforms the PLA file into an AIG file.
The PART classifier creates a set of rules that the developed software converts in an AAG file. After, the AIGER transforms the AAG file into an AIG file.

Requirements:
* Python 3.8;
* Java 8 or later;
* “abc.rc” at the same directory of “IWLS2020.py”;
* Compile abc-master at ~/tools/sources/abc-master;
* Copy the generated abc binary to ~/tools (with “abc” name);
* Compile “parttoaag” and “aigtoaig” or, if possible, give exec permission to respective binaries (chmod +x on Linux);
* At tools folder, it’s mandatory to naming binaries as: “abc”, “parttoaag” and “aigtoaig”;
* ABC tool source is at tools/sources, but the current project version can be view at: https://github.com/berkeley-abc/abc 

usage: `IWLS2020.py [-h] -i <file.pla> [-o </my/path/name.aig>] [-v] [--mltest pla_file1 [pla_fileN ...]]` 
`[--supress] [--persist] [--dont-clean-tmp]`

IWLS 2020 Contest AIG function predictor

optional arguments:
  `-h, --help            show this help message and exit`
    -i <file.pla>             Input PLA file path
    -o </my/path/name.aig>    Output AIG name
  -v, --verbose         Verbose mode, output all options tested by tool in the format
                        "<accuracy;;AIG_ANDs;;AIG_Depth;;runtime;;tmp_file>" (Default mode) or
                        "<accuracy;;AIG_ANDs;;mltest_file1;;...;;mltest_fileN;;AIG_Depth;;runtime;;tmp_file>"
                        (Mltest mode)
  ```
  --mltest pla_file1 [pla_fileN ...]  Especial mode to eval generated AIG against PLA files
  --supress                           Supress output messages
  --persist                           Persist all option files in persist_iwls folder
  --dont-clean-tmp                    Option for multiprocessing goal  
Usage example: python3.8 IWLS2020.py -i ex00.pla -o my_ex00.aig
```
