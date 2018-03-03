#!/usr/bin/python
import optparse
import logging
import sys
import re
import time
import datetime
import urllib, urllib2, cookielib
import json
import csv
import ast
import traceback
from collections import namedtuple
predictAddr='http://predict.habhub.org/ajax.php'
predictGetFormat=predictAddr+'?action=getCSV&uuid={}'
predictGetKMLFormat=predictAddr+'?action=getCSV&uuid={}'
predictSubmit=predictAddr+'?action=submitForm'
predictUIFormat='http://predict.habhub.org/#!/uuid={}'
ASCENT=5.6  #wznoszenie m/s
BURSTALT=33500 #wys pekniecia balonu
DRAG=10     #opadanie m/s

PosTuple=namedtuple('Pos','t lt ln at')

#logging.basicConfig(level=logging.DEBUG)

class HabHub(object):
 def __init__(self,posLat, posLon, posAlt, burstAlt=BURSTALT, ascent=ASCENT, drag=DRAG):
   logging.debug('init: {}/{} {} m. burst {} m. asc. {} desc -{}'.format(posLat, posLon, posAlt, burstAlt, ascent, drag))
   #,posLat, posLon, posAlt, burstAlt, ascent, drag)
   self.uuid=0
   self.httpGetPredict=""
   self.httpPredictUI=""
   self.burstPoint=PosTuple(0, 0.0, 0.0, 0.0)
   self.landPoint=PosTuple(0, 0.0, 0.0, 0.0)

   self.getPredictUuid (posLat, posLon, posAlt, burstAlt, ascent, drag)


 def getPredictUuid (self,posLat, posLon, posAlt, burstAlt, ascent, drag):
     logging.debug('getPredictUuid->start')
     (year,month,day,hour,min,second,wday,yday,isdst) = time.gmtime(time.time()+120) #ustawiamy czas w przyszlosc o 60 sekund
     logging.debug("godzina: {}-{}-{} {}:{}:{} {}/{} {}".format( year, month, day, hour, min, second, wday, yday, isdst))
     self.uuid=0
     try:
      self.cookieJar = cookielib.CookieJar()
      opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(self.cookieJar))
      urllib2.install_opener(opener)

      # acquire cookie
      req = urllib2.Request(predictAddr)
      rsp = urllib2.urlopen(req)

      # do POST
      paramsPost = dict(
                    launchsite='Other',
                    lat=posLat,
                    lon=posLon,
                    initial_alt=posAlt,
                    hour=hour,
                    min=min,
                    second=second,
                    day=day,
                    month=month,
                    year=year,
                    ascent=ascent,
                    burst=burstAlt,
                    drag=drag,
                    submit='Run+Prediction'
                  )
      paramsPostEnc = urllib.urlencode(paramsPost)
      logging.info(predictSubmit)
      logging.info(paramsPost)
      req = urllib2.Request(predictSubmit, paramsPostEnc)
      rsp = urllib2.urlopen(req)
      retJsonStr = rsp.read()

      retjson=json.loads(retJsonStr)
      logging.info(retjson['valid'])
      if (retjson['valid'] == 'true'):
         self.uuid= retjson['uuid']
         self.httpPredictUI=predictUIFormat.format(self.uuid)
         self.httpGetPredict=predictGetFormat.format(self.uuid)
         self.httpGetKMLPredict=predictGetKMLFormat.format(self.uuid)
         time.sleep(5)  #opoznienie aby serwis sie zupdateowal
     except Exception as e:
         logging.error("GetPredict - wystapil blad: {}".format(e))
     logging.debug('getPredictUuid->stop')

 def getPredictPath(self):
        logging.debug('getPredictPath->start')
        logging.debug('UID: {}'.format(self.uuid))
        try:
         if (self.uuid>0):
          logging.info(self.httpGetPredict)
          req = urllib2.Request(self.httpGetPredict)
          rsp = urllib2.urlopen(req)
          content = rsp.read()
          #print content
          predict=ast.literal_eval(content)

          for row in predict:
             ralt=0.0
             rtimestamp=0.0
             lastTimeStamp=0.0
             try:
              (rtimestamp,rlat,rlon,ralt)=tuple(row.split(','))
             except Exception as e:
              ralt=0.0
              #rtimestamp=0.0
              #lastTimeStamp=0.0
             #print ' '+rtimestamp+' '+ralt+' '+burstPoint.at;
             if (float(ralt)>float(self.burstPoint.at)):
                    self.burstPoint=PosTuple(
                                             datetime.datetime.fromtimestamp(float(rtimestamp)).strftime("%Y-%m-%d %H:%M:%S"),
                                             float(rlat),
                                             float(rlon),
                                             float(ralt)
                                            );
                    #print 'go up:'+ralt
             if (int(rtimestamp) > int(lastTimeStamp)):
                    lastTimeStamp=rtimestamp
                    self.landPoint=PosTuple(
                                            datetime.datetime.fromtimestamp(float(rtimestamp)).strftime("%Y-%m-%d %H:%M:%S"),
                                            float(rlat),
                                            float(rlon),
                                            float(ralt)
                                           );
        except Exception as e:
            logging.error("GetPredictPath - wystapil blad {}:".format(e))
            traceback.print_exc(file=sys.stdout)
        logging.info('getPredictPath->stop')



LOGGING_LEVELS = {'critical': logging.CRITICAL,
                  'error': logging.ERROR,
                  'warning': logging.WARNING,
                  'info': logging.INFO,
                  'debug': logging.DEBUG}

def main():
  parser = optparse.OptionParser()
  parser.add_option('-l', '--logging-level', help='Logging level')
  parser.add_option('-f', '--logging-file', help='Logging file name')
  (options, args) = parser.parse_args()
  logging_level = LOGGING_LEVELS.get(options.logging_level, logging.NOTSET)
  logging.basicConfig(level=logging_level, filename=options.logging_file,
                      format='%(asctime)s %(levelname)s: %(message)s',
                      datefmt='%Y-%m-%d %H:%M:%S')



  predict=HabHub(51.4113, 16.9025, 150)
  print('uuid {}'.format(predict.uuid))
  #predict.getPredictUuid(51.4113, 16.9025, 150,BURSTALT, ASCENT, DRAG);
  #print 'uuid {}s'.format(predict.uuid)
  predict.getPredictPath();
  print 'burst o {} ({} {}) {} m.'.format(
                                predict.burstPoint.t,
                                predict.burstPoint.lt,
                                predict.burstPoint.ln,
                                predict.burstPoint.at
                                               )
  print ' land o {} ({} {}) {} m.'.format(
                                predict.landPoint.t,
                                predict.landPoint.lt,
                                predict.landPoint.ln,
                                predict.landPoint.at
                                               )

if __name__ == '__main__':
   main()
