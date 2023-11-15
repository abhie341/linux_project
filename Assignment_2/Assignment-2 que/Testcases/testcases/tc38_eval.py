from os.path import exists
import os

file_exists = exists("tc38.out")
empty=(os.stat("tc38.out").st_size == 0)
if(not file_exists or empty):
    print("Failed")
    exit()

d = {"before":{"pmd":[],"pte":[]},"after":{"pmd":[],"pte":[]}}
with open("tc38.out","r") as f1:
    for line in f1:
        line = line.strip("\n")
        if(line == "before compaction"):
            flag = "before"
        if(line == "after compaction"):
            flag = "after"
        if("pmd" in line):
            line = line.split(",")
            l1 = [x.lstrip() for x in line]
            for i in l1:
                try:
                    loc = (i.split(":"))[0]
                    val = (i.split(":"))[1]
                    d[flag][loc].append(val)
                except:
                    print("Failed")
                    exit()

#print(d)
if(len(d["before"]["pmd"]) == 0 or len(d["after"]["pmd"]) == 0):
    print("Failed")
    exit()

result = any((x!=y and (int(y,base=16)>>7&1))for x,y in zip(d["before"]["pmd"], d["after"]["pmd"]))
if(result):
    print("Passed")
else:
    print("Failed")

