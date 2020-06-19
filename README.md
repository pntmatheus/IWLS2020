# IWLS2020
IWLS2020 Contest
The developed software reads a PLA file as input and creates an AIG file as output.  
We used Python as the principal programming language, but we also used JAVA and C languages as part of the solution.
To try to get the unknown boolean function, the proposed solution uses J48 and PART Artificial Intelligence classifiers.
We created a single file, combining both train and valid files, to use as input. The algorithm first transforms the PLA file in an ARFF to be handled by the WEKA tool. We used the WEKA tool to run five different configurations to the J48 classifier and also five configurations to the PART classifier, varying the confidence factor (“-C” option in WEKA).  It is applied cross-validation to decide what is the best option. We chose the best classifier and confidence option, and we used the minimum number of objects (“-M”) to test 6 more variants. Finally, we chose the best option. We used the ABC tool to check the size of the generated AIGs to match the contest requirements. 
The J48 classifier creates a decision tree that the developed software converts in a PLA file. In the sequence, the ABC tool transforms the PLA file into an AIG file.
The PART classifier creates a set of rules that the developed software converts in an AAG file. After, the AIGER transforms the AAG file into an AIG file.