# picoLog.py V0.8 221112 qrt@qland.de

import os, serial, time
from datetime import datetime
from matplotlib import pyplot as plt, dates
import matplotlib.ticker as ticker
import pandas as pd

PORT = 'COM9'                                                   # com port of Pico
DUMPFILE = 'dumpfile.dat'                                       # path and filename of dump file
XTICK_FREQU = 2                                                 # xtick freuqency in hours
XTICK_FORMAT = '%H:%M:%S'                                       #       format
AVS = 10                                                        # average sample factor

ser = 0

#-------------------------------------------------------------------------------

def logVis():
    if not os.path.exists(DUMPFILE):                                                  
        print('error: ' + DUMPFILE + ' dump file does not exist')
        return

    file = open(DUMPFILE, "r")                                  # open dump file
    osam = []                                                   # original samples
    dtin = []                                                   # date, time, interval, num words

    for line in file:
        if line.startswith('0x'):
            osam.extend([int(s[2:], 16) for s in line.split()])
        elif len(dtin) == 0:
            dtin = line.split()                                

    file.close()

    # - - - - - - - - - - - - - - - - - - - -

    dati = datetime.strptime(dtin[0] + dtin[1], '%Y%m%d%H%M%S') # get date and time
    interval = int(dtin[2])                                     #     sample interval

    asam = [ int(sum(osam[i:i+AVS])/AVS) for i in range(0, int(len(osam)/AVS)*AVS, AVS) ]       # averaged samples

    # - - - - - - - - - - - - - - - - - - - -

    dt = datetime.strftime(dati, '%Y-%m-%d %H:%M:%S')           # for use in DataFrame
    df = pd.DataFrame(dict(time=list(pd.date_range(dt, freq=pd.to_timedelta(interval*AVS, 'S'), periods=len(asam))), bright=asam))

    # - - - - - - - - - - - - - - - - - - - -

    fig, ax = plt.subplots()                                    # prepare plot
    fig.set_size_inches(10, 5)                                  # window size
    # fig.subplots_adjust(bottom=0.2)

    ax.set_xlabel('time', loc='right')                          # labels
    ax.set_ylabel('brightness', loc='top')

    ax.xaxis.set_major_formatter(dates.DateFormatter(XTICK_FORMAT))         # set xtick format
    ax.xaxis.set_major_locator(ticker.MultipleLocator(XTICK_FREQU / 24))    #           frequency

    ax.plot(df.time, df.bright)                                 # plot    
    
    plt.gcf().autofmt_xdate()                                   # beautify time stamps
    # plt.tick_params(rotation=45)

    plt.show()                                                  # show plot

#-------------------------------------------------------------------------------

def send(cmd, par=0, msg=True):
    ser.write(bytes('{} {}\n'.format(cmd, par), 'utf-8'))
    res = str(ser.readline(), 'utf-8').strip()
    
    if msg:
        print(res)

    if res != 'OK':
        print('error: {} failed'.format(cmd))

#-------------------------------------------------------------------------------

def sample():
    print('sampling ...')
    send('sample')

#-------------------------------------------------------------------------------

def dump():    
    try:
        file = open(DUMPFILE, 'w')
    except:
        print('error: cant write dumpfile')
        exit(1)

    print('dumping ...')
    ser.write(b'dump 0\n')
    n = 0

    while(True):
        line = str(ser.readline(), 'utf-8').strip()
        if len(line) == 0: break
        file.write(line + '\n')
        n += 1
        print('\r' + str(n), end='', flush=True)

    line = str(ser.readline(), 'utf-8').strip()
    file.write('\n' + line + '\n')

    print(' OK')
    file.close()

#-------------------------------------------------------------------------------

def visualize():
    logVis()

#-------------------------------------------------------------------------------

def remove():
    print('Delete sample data on Pico?')
    res = input('Y or N: ')

    if res.lower() == 'y':
        send('remove')

#-------------------------------------------------------------------------------

def format():
    print('Format flash on Pico?')
    res = input('Y or N: ')

    if res.lower() == 'y':
        send('format')

#-------------------------------------------------------------------------------

def adc():
    for i in range(8):
        ser.write(bytes('{} {}\n'.format('checkadc', 0), 'utf-8'))
        res = str(ser.readline(), 'utf-8').strip()
        print(res, end=' ' if (i+1) % 4 else '\n', flush=True)
        time.sleep(1)

    print('OK')

#-------------------------------------------------------------------------------

def setDate():
    print('Set Date (Enter for current date and time)')
    res = input('dd.mm.yyyy HH:MM:SS \n')

    if len(res):
        try:
            dati = datetime.strptime(res, '%d.%m.%Y %H:%M:%S') 
        except:
            print('error: input not valid')
            return
    else:
        dati = datetime.now().replace(microsecond=0)
        print('\033[A', end='')
        print(dati.strftime('%d.%m.%Y %H:%M:%S'))

    date = dati.year * 10000 + dati.month * 100 + dati.day
    time = dati.hour * 10000 + dati.minute * 100 + dati.second

    send('set_date', date, False)
    send('set_time', time)

#-------------------------------------------------------------------------------

def setInterval():
    print('Set Interval (5 s .. 24 h)')
    res = input('HH:MM:SS\n')

    hms = res.split(':')

    if len(hms) != 3:
        print('\033[Aerror: input not valid')
        return

    interval = int(hms[0]) * 3600 + int(hms[1]) * 60 + int(hms[2])

    if interval < 5:
        print('warning: interval set to 5 s')
        interval = 5

    if interval > 86400:
        print('warning: interval set to 24 h')
        interval = 86400

    send('set_interval', interval)

#-------------------------------------------------------------------------------

def setAppend():
    print('Set Append')
    res = input('ON or OFF\n').upper()

    if res!='ON' and res!='OFF':
        print('error: input not valid')
        return

    append = 0 if res=='OFF' else 1
    send('set_append', append)

#-------------------------------------------------------------------------------

def init():
    ser.write(bytes('test {}\n'.format(12345), 'utf-8'))
    res = str(ser.readline(), 'utf-8').strip()

#-------------------------------------------------------------------------------

def exitPgm():
    print('Exit')
    ser.close()
    exit(0)

#-------------------------------------------------------------------------------

try:
    ser = serial.Serial(PORT, 115200, timeout=1)
except:
    print('error: no port ' + PORT)
    exit(1)

init()                                      # send some data to reset buffers

while True:
    print()
    print('(s)ample     (d)ump           (v)isualize    (x)exit')
    print('(r)emove     (f)ormat         (a)dc')
    print('(1)set date  (2)set interval  (3)set append')
    
    res = input('>')    

    match res:
        case 's':
            sample()
        case 'd':
            dump()
        case 'v':
            visualize()
        case 'r':
            remove()
        case 'f':
            format()
        case 'a':
            adc()
        case '1':
            setDate()
        case '2':
            setInterval()
        case '3':
            setAppend()
        case 'x':
            exitPgm()
        case _:
            print('???')        
