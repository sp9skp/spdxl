#!/usr/bin/python
import sys
import math
import os
from collections import namedtuple
import time
import datetime

import optparse
import logging


scriptPath = os.path.realpath(os.path.dirname(sys.argv[0]))
os.chdir(scriptPath)
sys.path.append("py")

from libhabhub import HabHub


posTuple=namedtuple('pos','lat lon alt')
flyTuple=namedtuple('fly','burstAlt asc desc')


predictParamTuple=namedtuple('predictParam','pos fly1 fly2 fly3')
spaHourStartTuple=namedtuple('SpaHourStart','hour spaName predictParam')

wroclawPos    =posTuple(51.114, 16.882, 120)
legionowoPos  =posTuple(52.408, 20.956, 100)
lindenbergPos =posTuple(52.209, 14.122, 115)
prosciejowPos =posTuple(49.452, 17.135, 216)
greifswaldPos =posTuple(54.097, 13.406,   0)
pragaPos      =posTuple(50.008, 14.448, 420)


glinnikPos    =posTuple(49.452, 17.135, 216)
powidzPos     =posTuple(49.452, 17.135, 216)
rudniki       =posTuple(49.452, 17.135, 216)


maxFly    =flyTuple(33500, 5.6, 10)
stdFly    =flyTuple(32000, 5.6, 10)
lowFly    =flyTuple(28000, 5.6, 10)

chuteMaxFly  =flyTuple(33500, 5.2,  4)
chuteStdFly  =flyTuple(32000, 5.2,  4)
chuteLowFly  =flyTuple(28000, 5.2,  4)

maxOzoneFly    =flyTuple(33500, 5.6, 10)
stdOzoneFly    =flyTuple(32000, 5.6, 10)
lowOzoneFly    =flyTuple(28000, 5.6, 10)

chuteOzoneMaxFly  =flyTuple(33500, 5.2,  4)
chuteOzoneStdFly  =flyTuple(32000, 5.2,  4)
chuteOzoneLowFly  =flyTuple(28000, 5.2,  4)


flightList=[
   spaHourStartTuple('11:20', 'Wroclaw_12z',    predictParamTuple(wroclawPos, maxFly, stdFly, lowFly))
  ,spaHourStartTuple('23:20', 'Wroclaw_0z',    predictParamTuple(wroclawPos, maxFly, stdFly, lowFly))
  ,spaHourStartTuple('11:20', 'Legionowo_12z',  predictParamTuple(legionowoPos, maxFly, stdFly, lowFly))
  ,spaHourStartTuple('23:20', 'Legionowo_0z',  predictParamTuple(legionowoPos, maxFly, stdFly, lowFly))
  ,spaHourStartTuple('23:50', 'Lindenberg_0z', predictParamTuple(lindenbergPos, chuteMaxFly, chuteStdFly, chuteLowFly))
  ,spaHourStartTuple('05:50', 'Lindenberg_6z', predictParamTuple(lindenbergPos, chuteMaxFly, chuteStdFly, chuteLowFly))
  ,spaHourStartTuple('12:50', 'Lindenberg_12z', predictParamTuple(lindenbergPos, chuteMaxFly, chuteStdFly, chuteLowFly))
  ,spaHourStartTuple('17:50', 'Lindenberg_18z', predictParamTuple(lindenbergPos, chuteMaxFly, chuteStdFly, chuteLowFly))
  #,spaHourStartTuple('22:07', 'test', predictParamTuple(prosciejowPos, maxFly, stdFly, lowFly))
]


def predictFlight(spaName, lat, lon, alt, burstAlt, asc, desc):
  logging.debug('predictFlight {} {} {} m. burst {} m. asc. {} desc -{}'.format(lat, lon, alt, burstAlt, asc, desc))
  tekst=""
  predict=HabHub(lat, lon, alt, burstAlt, asc, desc)
  predict.getPredictPath();
  tekst+= 'burst,{},{},{},{},{},{}\n'.format(
                                spaName,
                                burstAlt,
                                predict.burstPoint.t,
                                predict.burstPoint.lt,
                                predict.burstPoint.ln,
                                predict.burstPoint.at
                                               )
  tekst+= 'land,{},{},{},{},{},{}\n'.format(
                                spaName,
                                burstAlt,
                                predict.landPoint.t,
                                predict.landPoint.lt,
                                predict.landPoint.ln,
                                predict.landPoint.at
                                               )

  return tekst

def cyclicPredict(predictParam,spaName):
  ### first predict
  tekst= predictFlight(spaName, predictParam.pos.lat, predictParam.pos.lon, predictParam.pos.alt, predictParam.fly1.burstAlt, predictParam.fly1.asc, predictParam.fly1.desc)
  tekst+=predictFlight(spaName, predictParam.pos.lat, predictParam.pos.lon, predictParam.pos.alt, predictParam.fly2.burstAlt, predictParam.fly2.asc, predictParam.fly2.desc)
  tekst+=predictFlight(spaName, predictParam.pos.lat, predictParam.pos.lon, predictParam.pos.alt, predictParam.fly3.burstAlt, predictParam.fly3.asc, predictParam.fly3.desc)
  return tekst

def cyclicClean(f):
  czas=format(datetime.datetime.now()+datetime.timedelta(hours=-3),"%Y-%m-%d %H:%M:%S")
  tekstn=""
  new_f=f.readlines()
  f.seek(0)
  for linia in new_f:
   dane=linia.split(",")
   if (dane[3] > czas):
     f.write(linia)
  f.truncate()
  return tekstn

def loopPredict(fileName):
  #print "loop"
  logging.debug('loopPredict: fileName {}'.format(fileName))
  tekst=""
  czasold=""
  if not os.path.isfile(fileName):
     logging.debug('loopPredict: fileName {} - not exists - create'.format(fileName))
     with open(fileName, 'w+') as f:
       f.closed
  while True:
   czas= datetime.datetime.fromtimestamp(time.time()).strftime("%H:%M")
   if ( czas <> czasold):
    logging.info('czas: {}'.format(czas))
    with open(fileName, 'r+') as f:
     cyclicClean(f)
     for flight in flightList:
       if ( flight.hour == czas ):
        logging.debug('start predict for point: {}/{}'.format(flight.predictParam.pos.lat,flight.predictParam.pos.lon))
        linia=cyclicPredict(flight.predictParam,flight.spaName)
        logging.debug('linia: {}'.format(linia))
        f.write(str(linia))
        tekst+=linia
     f.closed
     czasold=czas
  return


LOGGING_LEVELS = {'critical': logging.CRITICAL,
                  'error': logging.ERROR,
                  'warning': logging.WARNING,
                  'info': logging.INFO,
                  'debug': logging.DEBUG}


def main(args):
   #print "main"
   file=args[1]
   #if not os.path.isfile(file):
 
   parser = optparse.OptionParser()
   parser.add_option('-l', '--logging-level', help='Logging level')
   parser.add_option('-f', '--logging-file', help='Logging file name')
   (options, args) = parser.parse_args()
   logging_level = LOGGING_LEVELS.get(options.logging_level, logging.NOTSET)
   logging.basicConfig(level=logging_level, filename=options.logging_file,
                      format='%(asctime)s %(levelname)s: %(message)s',
                      datefmt='%Y-%m-%d %H:%M:%S')





   loopPredict(file)
   return

if __name__ == '__main__':
   main(sys.argv)


