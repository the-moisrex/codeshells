#!/usr/bin/env python3

# Source: https://github.com/AnonParsa/v2-checker/blob/main/v2checker.py

from requests import get
from base64 import b64decode
from json import loads
import socket, sys

def get_servers():
	global servers
	req = get('https://raw.githubusercontent.com/mahdibland/ShadowsocksAggregator/master/sub/splitted/vmess.txt')
	for s in req.text.splitlines():
		servers.append(s)

def check_servers():
	for s in servers:
		js = loads(b64decode(s.replace("vmess://", "")).decode())
		add = js['host'] if js['host'] else js['add']
		port = js['port']
		so = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		try: so.connect((add, port))
		except KeyboardInterrupt: sys.exit(1)
		except: pass
		else: print(s)

if __name__ == '__main__':
	servers = []
	get_servers()
	check_servers()
