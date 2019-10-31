import os
import glob
import subprocess
from multiprocessing import Pool
benchs = list(glob.glob('./bench/*.c'))
#os.mkdir('/tmp/bench')

def run(fn):
    print("cc " + fn)
    subprocess.run(['eopcc', fn, '-o', '/tmp/'+fn+'.out'])
    print("sim " + fn)
    subprocess.run(['eopsim', '/tmp/'+fn+'.out', fn+'.report'])
    print(fn + " fin")

with Pool(6) as p:
  p.map(run, benchs)
