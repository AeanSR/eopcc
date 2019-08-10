from __future__ import print_function

def work(f, items):
    f.write("%d\n" % len(items))
    for item in items:
        print (len(item), item)
        f.write("%d\n" % len(item))
        for index, i_ in enumerate(list(item)):
            print (index, i_)
            if index < 2:
                f.write("%s\n" % i_)
                continue
            if i_.startswith('#'):
                f.write("0 %s\n" % i_[1:])
                continue
            if i_.startswith('ptr'):
                if ('#' in i_):
                    f.write("3 %s\n" % i_[5:])
                    continue
                else:
                    f.write("2 %s\n" % i_[5:])
                    continue
            if i_.startswith('r'):
                f.write("1 %s\n" % i_[1:])
                continue
            if i_.startswith('!'):
                f.write("4 %s\n" % i_[1:])
                continue
            print ("ERROR")

    
items = [l.strip().split() for l in open("./eop.out").readlines()]
f1 = open("eop.output1", "w")
f2 = open("eop.output2", "w")

pos1 = items.index(['main:'])
pos2 = items.index(['except:'])
items1 = items[pos1+1 : pos2]
items2 = items[pos2+1 : ]

work(f1, items1)
work(f2, items2)
