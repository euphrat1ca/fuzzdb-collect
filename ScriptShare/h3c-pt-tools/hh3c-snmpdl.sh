#!/bin/bash

# Download Huawei/H3C configurations via SNMP to TFTP or FTP
#
# Use the "-o" option to force old H3C/Huawei SNMP OID
#
# If "-u value" is specified it is assumed the download method
# is FTP
#
# Author: Kurt Grutzmacher <grutz at jingojango.net>
# License: BSD

function usage()
{
  cat << EOF

Huawei/H3C SNMP downloader
--------------------------

Usage:

   $0 -i device-ip -s server-ip [-u ftp-user -p ftp-pass] [-o] [-c community]

   -i option      Device IP Address (required)
   -s option      Server IP Address (required)
   -u option      FTP Username (if using FTP)
   -p option      FTP Password (if using FTP)
   -o             Use old 2011.20 SNMP OID
   -c option      SNMP Community (default: private)

EOF
}

snmpset="`which snmpset` -r 0 -t 1 -v 1"
variant="25506"
comm="private"

while getopts "i:s:u:p:oc:" opt; do
  case $opt in
    i)
      ip=$OPTARG
      ;;
    s)
      destip=$OPTARG
      ;;
    u)
      user=$OPTARG
      ;;
    p)
      pw=$OPTARG
      ;;
    o)
      variant="2011.10"
      ;;
    c)
      comm=$OPTARG
      ;;
    h)
      usage
      exit 1
      ;;
  esac
done

if [ -z $ip ] ; then
  echo "[!] Must provide a Device IP address"
  usage
  exit 1
fi

if [ -z $destip ] ; then
  echo "[!] Must provide a TFTP/FTP Server IP address"
  usage
  exit 1
fi

echo Collecting configuration from $ip

# purge any existing configuration
$snmpset -c $comm $ip 1.3.6.1.4.1.$variant.2.4.1.2.4.1.9.1 i 6
sleep 1

if [ -n $user ] ; then
  $snmpset -c $comm $ip 1.3.6.1.4.1.$variant.2.4.1.2.4.1.2.1 i 3 \
  1.3.6.1.4.1.$variant.2.4.1.2.4.1.3.2 i 1 \
  1.3.6.1.4.1.$variant.2.4.1.2.4.1.4.2 s $ip-confg \
  1.3.6.1.4.1.$variant.2.4.1.2.4.1.5.2 a $destip \
  1.3.6.1.4.1.$variant.2.4.1.2.4.1.6.2 s $user \
  1.3.6.1.4.1.$variant.2.4.1.2.4.1.7.2 s $pw \
  1.3.6.1.4.1.$variant.2.4.1.2.4.1.9.2 i 4
else
  $snmpset -c $comm $ip 1.3.6.1.4.1.$variant.2.4.1.2.4.1.2.1 i 3 \
  1.3.6.1.4.1.$variant.2.4.1.2.4.1.3.2 i 2 \
  1.3.6.1.4.1.$variant.2.4.1.2.4.1.4.2 s $ip-confg \
  1.3.6.1.4.1.$variant.2.4.1.2.4.1.5.2 a $destip \
  1.3.6.1.4.1.$variant.2.4.1.2.4.1.9.2 i 4

fi

# purge this record
sleep 10
$snmpset -c $comm $ip 1.3.6.1.4.1.$variant.2.4.1.2.4.1.9.1 i 6

