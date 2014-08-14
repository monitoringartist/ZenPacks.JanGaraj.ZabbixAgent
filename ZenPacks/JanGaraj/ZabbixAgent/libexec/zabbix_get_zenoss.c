/*
** Zabbix get utility for Zenoss (C version)
** Output format is prepared for Zenoss JSON parser
** Source code is just slightly modified zabbix_get source code,
** so you will need zabbix source for compilation.
** Jan Garaj - www.jangaraj.com
**
** Zabbix
** Copyright (C) 2001-2014 Zabbix SIA
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
**/
#include "threads.h"
#include "comms.h"
#include "zbxgetopt.h"

char		*key = NULL;
const char	*progname = NULL;
const char	title_message[] = "Zabbix get for Zenoss";
const char	syslog_app_name[] = "zabbix_get_zenoss";
const char	usage_message[] = "[-hV] -s <host name or IP> [-p <port>] [-I <IP address>] -k <key> -c <component> -d <datapoint>";

const char	*help_message[] = {
	"Options:",
	"  -s --host <host name or IP>          Specify host name or IP address of a host",
	"  -p --port <port number>              Specify port number of agent running on the host. Default is 10050",
	"  -I --source-address <IP address>     Specify source IP address",
	"",
	"  -k --key <key of metric>             Specify key of item to retrieve value for",
	"",
	"  -c --component <name>                Zenoss component name. Default is empty string",
	"  -d --datapoint <name>                Zenoss datapoint name",
	"",
	"  -h --help                            Give this help",
	"  -V --version                         Display version number",
	"",
	"Example: zabbix_get_zenoss -s 127.0.0.1 -p 10050 -d system.cpu.load.all.avg1 -k \"system.cpu.load[all,avg1]\"",
	NULL	/* end of text */
};

/* COMMAND LINE OPTIONS */

/* long options */
struct zbx_option	longopts[] =
{
	{"host",		1,	NULL,	's'},
	{"port",		1,	NULL,	'p'},
	{"key",			1,	NULL,	'k'},
	{"source-address",	1,	NULL,	'I'},
	{"component",		1,	NULL,	'c'},
	{"datapoint",		1,	NULL,	'd'},
	{"help",		0,	NULL,	'h'},
	{"version",		0,	NULL,	'V'},
	{NULL}
};

/* short options */
static char     shortopts[] = "s:p:k:I:c:d:hV";

/* end of COMMAND LINE OPTIONS */

#if !defined(_WINDOWS)

/******************************************************************************
 *                                                                            *
 * Function: get_signal_handler                                               *
 *                                                                            *
 * Purpose: process signals                                                   *
 *                                                                            *
 * Parameters: sig - signal ID                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	get_signal_handler(int sig)
{
	if (SIGALRM == sig)
		printf("{\"events\":[{\"severity\":4,\"summary\":\"Zabbix metric error %s: Timeout while executing operation\",\"eventClass\":\"/Status/ZabbixAgent\"}]}", key);

	exit(EXIT_FAILURE);
}

#endif /* not WINDOWS */

/******************************************************************************
 *                                                                            *
 * Function: get_value                                                        *
 *                                                                            *
 * Purpose: connect to Zabbix agent, receive and print value                  *
 *                                                                            *
 * Parameters: host - server name or IP address                               *
 *             port - port number                                             *
 *             key  - item's key					      *
 *	       component - Zenoss component name                              *
 *	       datapoint - Zenoss datapoint name			      *
 *                                                                            *
 ******************************************************************************/
static void	get_value(const char *source_ip, const char *host, unsigned short port, const char *key, const char *component, const char *datapoint)
{
	zbx_sock_t	s;
	int		ret;
	char		request[1024];

	if (SUCCEED == (ret = zbx_tcp_connect(&s, source_ip, host, port, GET_SENDER_TIMEOUT)))
	{
		zbx_snprintf(request, sizeof(request), "%s\n", key);

		if (SUCCEED == (ret = zbx_tcp_send(&s, request)))
		{
			if (SUCCEED == (ret = SUCCEED_OR_FAIL(zbx_tcp_recv_ext(&s, ZBX_TCP_READ_UNTIL_CLOSE, 0))))
			{
				if (0 == strcmp(s.buffer, ZBX_NOTSUPPORTED))
				{
					zbx_rtrim(s.buffer + sizeof(ZBX_NOTSUPPORTED), "\r\n");
					printf("{\"events\":[{\"severity\":4,\"summary\":\"Zabbix metric %s: ZBX_NOTSUPPORTED\",\"message\":\"%s\",\"eventClass\":\"/Status/ZabbixAgent\",\"eventKey\":\"%s\"}]}", key, s.buffer, key);
				}
				else
				{
					zbx_rtrim(s.buffer, "\r\n");
					printf("{\"values\":{\"%s\":{\"%s\":%s}},\"events\":[{\"severity\":0,\"summary\":\"Clearing previous problems\",\"message\":\"Clearing previous problems\",\"eventClass\":\"/Status/ZabbixAgent\",\"eventKey\":\"%s\"},{\"severity\":0,\"summary\":\"Clearing previous problems\",\"message\":\"Clearing previous problems\",\"eventClass\":\"/Status/ZabbixAgent\",\"eventKey\":\"connection\"}]}", component, datapoint, s.buffer, key);
				}
			}
		}

		zbx_tcp_close(&s);
	}

	if (FAIL == ret)
		printf("{\"events\":[{\"severity\":4,\"summary\":\"Connection error to %s:%d\",\"message\":\"%s\",\"eventClass\":\"/Status/ZabbixAgent\",\"eventKey\":\"connection\"}]}", host, port, zbx_tcp_strerror());
}

/******************************************************************************
 *                                                                            *
 * Function: main                                                             *
 *                                                                            *
 * Purpose: main function                                                     *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int	main(int argc, char **argv)
{
	unsigned short	port = 10050;
	int		ret = SUCCEED;
	char		*host = NULL, *source_ip = NULL, *component = NULL, *datapoint = NULL, ch;

	progname = get_program_name(argv[0]);

	/* parse the command-line */
	while ((char)EOF != (ch = (char)zbx_getopt_long(argc, argv, shortopts, longopts, NULL)))
	{
		switch (ch)
		{
			case 'k':
				key = strdup(zbx_optarg);
				break;
			case 'c':
				component = strdup(zbx_optarg);
				break;
			case 'd':
				datapoint = strdup(zbx_optarg);
				break;
			case 'p':
				port = (unsigned short)atoi(zbx_optarg);
				break;
			case 's':
				host = strdup(zbx_optarg);
				break;
			case 'I':
				source_ip = strdup(zbx_optarg);
				break;
			case 'h':
				help();
				exit(EXIT_SUCCESS);
				break;
			case 'V':
				version();
				exit(EXIT_SUCCESS);
				break;
			default:
				usage();
				exit(EXIT_FAILURE);
				break;
		}
	}

	if (NULL == host || NULL == key)
	{
		usage();
		ret = FAIL;
	}

	if (NULL == component)
	{
		component = strdup("");
	}

	if (SUCCEED == ret)
	{
#if !defined(_WINDOWS)
		signal(SIGINT,  get_signal_handler);
		signal(SIGTERM, get_signal_handler);
		signal(SIGQUIT, get_signal_handler);
		signal(SIGALRM, get_signal_handler);
#endif
		get_value(source_ip, host, port, key, component, datapoint);
	}

	zbx_free(host);
	zbx_free(key);
	zbx_free(component);
	zbx_free(datapoint);

	return ret;
}
