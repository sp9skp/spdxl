import os, sys, string
import datetime
import urllib


today = datetime.datetime.now()


def GetFtpPath(day_offset):
        res = ''
        res += 'ftp://cddis.gsfc.nasa.gov/gps/data/daily/'
        res += str( datetime.datetime.now().timetuple().tm_year )
        res += "/brdc/brdc"
        res += string.zfill( ( datetime.datetime.now().timetuple().tm_yday + day_offset ), 3 )
        res += '0.'
        res += str( datetime.datetime.now().timetuple().tm_year )[2:]
        res += 'n.Z'
        return res


def GetFtpPathDe(day_offset):
        res = ''
        res += 'ftp://igs.bkg.bund.de/IGS/BRDC/'
        res += str( datetime.datetime.now().timetuple().tm_year )
        res += "/"
        res += today.strftime('%j')
        res += "/brdc"
        res += string.zfill( ( datetime.datetime.now().timetuple().tm_yday + day_offset ), 3 )
        res += '0.'
        res += str( datetime.datetime.now().timetuple().tm_year )[2:]
        res += 'n.Z'
        return res


def FtpGet(remoteFile, out):
        urllib.urlretrieve('ftp://server/path/to/file', out)

def main():
        remoteFile = GetFtpPath(0)
        print remoteFile
        try:
                urllib.urlretrieve( remoteFile, '/tmp/rinex.Z' )
        except:
                remoteFile = GetFtpPath(-1)
                print remoteFile
                urllib.urlretrieve( remoteFile, '/tmp/rinex.Z' )

        try:
                os.system('7z e ' + '/tmp/rinex.Z -aoa')
                os.system('rm /tmp/rinex.Z')
        except:
                print "extraction requires cygwin"


if __name__ == '__main__':
        main()


