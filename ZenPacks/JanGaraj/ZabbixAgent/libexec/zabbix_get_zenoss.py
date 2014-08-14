#!/usr/bin/env python

"""
** Zabbix get utility for Zenoss (python version) 
** - output format is prepared for Zenoss JSON parser 
** Copyright (C) 2014 Jan Garaj - www.jangaraj.com
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
"""

from optparse import OptionParser, TitledHelpFormatter
from ConfigParser import SafeConfigParser
import socket
import struct
import sys

VERSION = "0.0.1"
moptions = ('host', 'key', 'component', 'datapoint')
# zabbix_get constants
GET_SENDER_TIMEOUT = 60
ZBX_NOTSUPPORTED =	"ZBX_NOTSUPPORTED"
ZBX_MEBIBYTE = 1048576
ZBX_MAX_RECV_DATA_SIZE = (128 * ZBX_MEBIBYTE)

def buildParams(cfg, parser):
    """ Add options """
    parser.add_option('-s', '--host', dest='host', type="string", help='*Specify host name or IP address of a host')
    parser.add_option('-k', '--key', dest='key', type="string", help='*Specify key of item to retrieve value for')
    parser.add_option('-d', '--datapoint', dest='datapoint', type="string", help='*Zenoss datapoint name')
    parser.add_option('-c', '--component', dest='component', type="string", help='Zenoss component name. Default is empty string', default='')
    parser.add_option('-p', '--port', dest='port', type="int", help='Specify port number of agent running on the host. Default is 10050', default=10050)

    return parser

def readCommadLine(arguments, usage):
    """Read the command line -  returns options"""
    parser = OptionParser(usage, version="%s" % VERSION, formatter=TitledHelpFormatter(width=255, indent_increment=4))
    cfg = SafeConfigParser()
    buildParams(cfg, parser)
    options, args = parser.parse_args(arguments)
    for option in moptions:
        if getattr(options, option) is None:
            print "\nA mandatory option is missing\n"
            parser.print_help()
            sys.exit(1)    
            
    return options

def str2packed(data):
    header_field =  struct.pack('<4sBQ', 'ZBXD', 1, len(data))
    return header_field + data

def packed2str(packed_data):
    header, version, length = struct.unpack('<4sBQ', packed_data[:13])
    (data, ) = struct.unpack('<%ds'%length, packed_data[13:13+length])
    return data

def zabbix_get(options):
    """Call zabbix agent and print value/problems"""
    # IPv4 validation
    try:
        socket.inet_pton(socket.AF_INET, options.host)
        conoptions = [((socket.AF_INET), (options.host, options.port))]        
    except socket.error:
        # IPv6 validation
        try:
            socket.inet_pton(socket.AF_INET6, options.host)
            conoptions = [((socket.AF_INET6), (options.host, options.port))]
        except socket.error:    
            # host is not valid IP - try to resolve hostname
            try:
                conoptions = [(o[0], o[4]) for o in socket.getaddrinfo(options.host, options.port, socket.AF_UNSPEC, socket.SOCK_STREAM)]                                
            except socket.gaierror, err:
                print '{"events":[{"severity":4,"summary":"Can\'t resolve hostname %s","message":"%s","eventClass":"/Status/ZabbixAgent","eventKey":"connection"}]}' % (options.host, err[1])
                sys.exit(1)
            except socket.herror, err:
                print '{"events":[{"severity":4,"summary":"Address-related error for hostname %s","message":"%s","eventClass":"/Status/ZabbixAgent","eventKey":"connection"}]}' % (options.host, err[1])
                sys.exit(1)
    
    family, hostport = conoptions[0]
    s = socket.socket(family, socket.SOCK_STREAM)
    s.settimeout(GET_SENDER_TIMEOUT)
    try:    
        s.connect(hostport)            
    except socket.timeout:
        print '{"events":[{"severity":4,"summary":"Connection timeout to %s","message":"Current timeout setting is %s","eventClass":"/Status/ZabbixAgent","eventKey":"connection"}]}' % (options.host, GET_SENDER_TIMEOUT)
        sys.exit(1)
    except socket.error, err:
        print '{"events":[{"severity":4,"summary":"Connection error to %s:%s","message":"%s","eventClass":"/Status/ZabbixAgent","eventKey":"connection"}]}' % (options.host, options.port, err[1])
        sys.exit(1)    

    s.sendall(str2packed(options.key))

    data = ''
    while True:
        buff = s.recv(1024)
        if not buff:
            break
        data += buff
    
    data = packed2str(data)
    if data == ZBX_NOTSUPPORTED:
        print '{"events":[{"severity":4,"summary":"Zabbix metric %s: ZBX_NOTSUPPORTED","message":"%s","eventClass":"/Status/ZabbixAgent","eventKey":"%s"}]}' % (options.key, data, options.key)
    else:    
        print '{"values":{"%s":{"%s":%s}},"events":[{"severity":0,"summary":"Clearing previous problems","message":"Clearing previous problems","eventClass":"/Status/ZabbixAgent","eventKey":"%s"},{"severity":0,"summary":"Clearing previous problems","message":"Clearing previous problems","eventClass":"/Status/ZabbixAgent","eventKey":"connection"}]}' % (options.component, options.datapoint, data, options.key)
    s.close()
    return 0

def main(arguments):
    """The main function"""

    usage = """
    %prog [options]

    Zabbix get utility for Zenoss - output format is prepared for Zenoss JSON parser.

    Example: zabbix_get_zenoss.py -s 127.0.0.1 -d system.cpu.load.all.avg1 -k \"system.cpu.load[all,avg1]\""""

    conf = readCommadLine(arguments, usage)
    return zabbix_get(conf)

if __name__ == '__main__':
    main(sys.argv[1:])