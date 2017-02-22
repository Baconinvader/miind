import os
import visualize
import sys
import time
import subprocess
 
MONITOR=os.path.dirname(os.path.realpath(__file__)) 

def processlist(modelname):
    filelist=subprocess.check_output(["ls",MONITOR + '/' + modelname + '_mesh']).split()
    sortlist=sorted(filelist, key=lambda item: (float(item.split('_')[-2]) ))
    f=open('processlist.txt','w')
    for fi in sortlist:
        f.write('file \'' + fi + '.png\'\n' )

def loop(modelname,option):
    m=visualize.ModelVisualizer(modelname)

    i = 0
    while(1):
    
        filelist=subprocess.check_output(["ls",MONITOR + '/' + modelname + '_mesh']).split()
        # nice, but filelist is ordered alphabetically. want sort on time which we expect (!) is the last field
        sortlist=sorted(filelist, key=lambda item: (float(item.split('_')[-2]) ))
        print sortlist
        if (i == len(sortlist) ): break
        if (len(sortlist) > 0):
            timestring = sortlist[i].split('_')[-2]
            print sortlist[i], timestring
            if (option == ''):
                m.showfile(MONITOR + '/' + modelname + '_mesh' + '/' + sortlist[i], runningtext = 't = ' + timestring)
            else:
                m.showfile(MONITOR + '/' + modelname + '_mesh' + '/' + sortlist[i], pdfname=sortlist[i], runningtext = 't = ' + timestring)
                
            i+=1

if __name__ == "__main__":
    if len(sys.argv) == 2:
        loop(sys.argv[1],'')
    else:
        loop(sys.argv[1],'y')
