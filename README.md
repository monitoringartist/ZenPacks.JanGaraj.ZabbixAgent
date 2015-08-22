ZenPacks.JanGaraj.ZabbixAgent
=============================

About
=====

This ZenPack unlocks Zabbix agent functionality in Zenoss. The best benefit is 
that Zabbix agent is available for wide range of OS (UNIX, Windows, ...). Default 
Zabbix templates adapted for Zenoss are included.

Please donate to author, so he can continue to publish other awesome projects 
for free:

[![Paypal donate button](http://jangaraj.com/img/github-donate-button02.png)]
(https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=8LB6J222WRUZ4)

**Please test it and provide your feedback.**   
![ZabbixOSLinux Template graphs](https://raw.githubusercontent.com/monitoringartist/ZenPacks.JanGaraj.ZabbixAgent/master/image_ZabbixOSLinux-Graphs1.png)                                 

Requirements
============

Zenoss
------

You must first have, or install, Zenoss 4. This ZenPack was tested against 
Zenoss 4.2.5. You can download the free Core version of Zenoss 
from http://community.zenoss.org/community/download.

Zabbix agent
------------

Visit [Zabbix download page](http://www.zabbix.com/download.php) and choose the right free pre-compiled Zabbix agent version.
Or if package `zabbix-agent` exists in your distribution repository, you can use it. 
ZenPack doesn't have any condition for Zabbix agent version. 
Please check [Zabbix documentation](https://www.zabbix.com/documentation) if you have a problem with installation.     
Please configure also agent (usually zabbix_agentd.conf file). Edit line with `Server` config:

`Server=<IP_OF_ZENOSS_COLLECTOR(S)>`

Incoming connections are accepted only from the hosts listed in this settings. Then (re)start agent.

Network
-------

Zenoss collector/master must have access to Zabbix agents (default port 10050) on monitored devices.

Installation
============

Normal Installation (packaged egg)
----------------------------------

Download the egg file.
Copy this file to your Zenoss server and run the following commands as the zenoss
user.

```
zenpack --install ZenPacks.JanGaraj.ZabbixAgent-0.7.0.egg
zenoss restart
```
        

Developer Installation (link mode)
----------------------------------

If you wish to further develop and possibly contribute back to the ZabbixAgent
ZenPack you should clone the [git repository]
(https://github.com/monitoringartist/ZenPacks.JanGaraj.ZabbixAgent.git),
then install the ZenPack in developer mode using the following commands.

```
git clone git://github.com/monitoringartist/ZenPacks.JanGaraj.ZabbixAgent.git
zenpack --link --install ZenPacks.JanGaraj.ZabbixAgent
zenoss restart
```

zProperties
===========

ZenPack creates zProperty *zZabbixPort* with default value 10050 (default Zabbix 
agent port). If your Zabbix agent is configured with another listen port, 
change *zZabbixPort* accordingly.  

Usage
=====

Go to the specific device (yes, it needs to already be added) and bind right 
template(s).

Installing the ZenPack will add the following items to your Zenoss system:

Monitoring Templates
--------------------

Templates are "imported" from Zabbix and adapted for Zenoss, because Zenoss 
doesn't support all Zabbix features (especially non numeric metric). Please bear 
in mind, that CPU is default component template for all Devices. 
Set it up only for devices, where Zabbix Agent is installed.

- /Devices/rrdTemplates/CPU (Component Template)
- /Devices/rrdTemplates/ZabbixAppFTPService
- /Devices/rrdTemplates/ZabbixAppHTTPService
- /Devices/rrdTemplates/ZabbixAppHTTPSService
- /Devices/rrdTemplates/ZabbixAppIMAPService
- /Devices/rrdTemplates/ZabbixAppLDAPService
- /Devices/rrdTemplates/ZabbixAppMySQL
- /Devices/rrdTemplates/ZabbixAppNNTPService
- /Devices/rrdTemplates/ZabbixAppNTPService
- /Devices/rrdTemplates/ZabbixAppPOPService
- /Devices/rrdTemplates/ZabbixAppSMTPService
- /Devices/rrdTemplates/ZabbixAppSSHService
- /Devices/rrdTemplates/ZabbixAppTelnetService
- /Devices/rrdTemplates/ZabbixAppZabbixAgent
- /Devices/rrdTemplates/ZabbixOSAIX
- /Devices/rrdTemplates/ZabbixOSFreeBSD
- /Devices/rrdTemplates/ZabbixOSHP-UX
- /Devices/rrdTemplates/ZabbixOSLinux
- /Devices/rrdTemplates/ZabbixOSMacOSX
- /Devices/rrdTemplates/ZabbixOSOpenBSD
- /Devices/rrdTemplates/ZabbixOSSolaris
- /Devices/rrdTemplates/ZabbixOSWindows 

Event Classes
-------------

- /Events/Status/ZabbixAgent

Creatig a custom metric
=======================

You can use all metric provided by Zabbix agent in passive mode. For example all 
file relative metrics (e.g. log, logrt) are available only in active mode. Check 
Zabbix agent documentation - [available item keys]
(https://www.zabbix.com/documentation/2.2/manual/config/items/itemtypes/zabbix_agent#supported_item_keys)

Summary:
- create new Datasource, type COMMAND (e.g. name: 'CPU steal time')
- create new Datapoint for your new datasource, only one datapoint per datasource 
is supported (e.g. name: 'system.cpu.util.steal')
- edit your new Datasource:
Parser must be *ZenPacks.JanGaraj.ZabbixAgent.parsers.ZabbixAgentJSON*.

Command template must call provided zabbix_get_zenoss utility with the right parameters: 
```
${here/ZenPackManager/packs/ZenPacks.JanGaraj.ZabbixAgent/path}/libexec/zabbix_get_zenoss -s ${device/id} -p ${here/zZabbixPort} -k "system.cpu.util[,nice]" -d "system.cpu.util.steal" -c ""
```

Parameters:

-s ${device/id} - host name or IP of monitored device, I recommend to use 
${device/id} as default value

-p ${here/zZabbixPort} - Zabbix agent port on monitored device, zProperty 
zZabbixPort (with default value 10050) is used here

-k "system.cpu.util[,steal]" - Zabbix item key, see Zabbix manual for parameters 
and available keys

-d "system.cpu.util.steal" - Zenoss datapoint name

-c "" - Zenoss component id, use it in component template, otherwise you can omit 
this parameter (default value is "" - no component)

Extending Zabbix agent metric
============================= 
  
You can define your own metric, if some metric, which you need is not provided 
by Zabbix agent by default. Please refer to Zabbix manual:

- userparameters

https://www.zabbix.com/documentation/2.2/manual/config/items/userparameters

- loadable modules (Unix only)

https://www.zabbix.com/documentation/2.2/manual/config/items/loadablemodules

Screenshots
===========

A few graph screenshots from device with ZabbixOSLinux template for example:
![ZabbixOSLinux Template graphs](https://raw.githubusercontent.com/monitoringartist/ZenPacks.JanGaraj.ZabbixAgent/master/image_ZabbixOSLinux-Graphs2.png)
![ZabbixOSLinux Template graphs](https://raw.githubusercontent.com/monitoringartist/ZenPacks.JanGaraj.ZabbixAgent/master/image_ZabbixOSLinux-Graphs3.png)
![ZabbixOSLinux Template graphs](https://raw.githubusercontent.com/monitoringartist/ZenPacks.JanGaraj.ZabbixAgent/master/image_ZabbixOSLinux-Graphs4.png)

Performance notes
=================

ZenPacks.JanGaraj.ZabbixAgent 0.7.0 use one TCP call/connection per metric. It's not 
very efficient, so this call should be very quick. ZenPacks.JanGaraj.ZabbixAgent 
libexec folders contains also C version of zabbix_get_zenoss utility for better 
perfomance. Default used version is python:

```
[root@device libexec]# time ./zabbix_get_zenoss.py -s zabbix -d system.cpu.load.all.avg1 -k "system.cpu.load[all,avg1]"
{"values":{"":{"system.cpu.load.all.avg1":0.000000}},"events":[{"severity":0,"summary":"Clearing previous problems","message":"Clearing previous problems","eventClass":"/Status/ZabbixAgent","eventKey":"system.cpu.load[all,avg1]"},{"severity":0,"summary":"Clearing previous problems","message":"Clearing previous problems","eventClass":"/Status/ZabbixAgent","eventKey":"connection"}]}

real    0m0.079s
user    0m0.043s
sys     0m0.033s
```

For performance testing purpose, also C version has been created. You will need 
to compile it, what can be challenge, so some precompiled version 
(Ubuntu, CentOS, ...) are provided.

Performance test with compiled version:
```
[root@device libexec]# time ./zabbix_get_zenoss_centos5 -s zabbix -d system.cpu.load.all.avg1 -k "system.cpu.load[all,avg1]"
{"values":{"":{"system.cpu.load.all.avg1":0.000000}},"events":[{"severity":0,"summary":"Clearing previous problems","message":"Clearing previous problems","eventClass":"/Status/ZabbixAgent","eventKey":"system.cpu.load[all,avg1]"},{"severity":0,"summary":"Clearing previous problems","message":"Clearing previous problems","eventClass":"/Status/ZabbixAgent","eventKey":"connection"}]}
real    0m0.007s
user    0m0.000s
sys     0m0.007s
```
  
Generally compiled version is 10x more faster than python version in this 
synthetic performance test. So let's go to check performance with zencommand 
(zencommand run -d zabbix -v 10). Tested device is zabbix VM with binded 
ZabbixOSLinux template and with 33 datapoints.

0.6 second takes RUNNING->IDLE for compiled C version of zabbix_get_zenoss version:
```
2014-08-09 14:01:27,832 DEBUG zen.collector.scheduler: Task zabbix 60 Local changing state from RUNNING to FETCH_DATA
2014-08-09 14:01:28,426 DEBUG zen.collector.scheduler: Task zabbix 60 Local changing state from STORE_PERF_DATA to IDLE
```

3.2 seconds takes RUNNING->IDLE for python version of zabbix_get_zenoss version:
```
2014-08-09 14:06:50,461 DEBUG zen.collector.scheduler: Task zabbix 60 Local changing state from RUNNING to FETCH_DATA
2014-08-09 14:06:53,702 DEBUG zen.collector.scheduler: Task zabbix 60 Local changing state from STORE_PERF_DATA to IDLE
```

I've made also test with one second CPU utilization pooling for one device and 
it was 10% CPU utilization for python version vs 1% CPU utilization for 
compiled C version. Graph of CPU utilization, when one second pooling was active: 
![One second CPU utilization pooling]
(https://raw.githubusercontent.com/monitoringartist/ZenPacks.JanGaraj.ZabbixAgent/master/image_CPU_utilization_one_second_pooling.png)


Conclusion:

Python is amazing option for developers. I love "import pdb; pdb.set_trace();" 
feature for python debugging also in Zenoss. But if speed is critical for you, 
then consider binaries (compiled C code).

TODO list
=========

- implement Zabbix agent datasource
- implement daemon zenzabbixserver for active/passive agent mode and better performance
- implement Zabbix modeller Filesystem/Interface plugin (low level discovery functionality of Zabbix agent)
- improve speed/perfomance (cython, pypy, ...)
  
Author
======

[Devops Monitoring zExpert](http://www.jangaraj.com), who loves monitoring 
systems, which start with letter Z. Those are Zabbix and Zenoss.

Professional monitoring services:

[![Monitoring Artist](http://monitoringartist.com/img/github-monitoring-artist-logo.jpg)]
(http://www.monitoringartist.com)
