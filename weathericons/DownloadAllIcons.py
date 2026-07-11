"""
Download png files from OpenWeatherMap URL.
Saves png files in the ./png directory.

Usage:
  python geticons.py --delay 0.1 --timeout 10
"""
import os
import time
import argparse
import sys
import requests
from urllib import request as urlrequest
from urllib.error import HTTPError, URLError

downloaddir = "png"

def download_one(name, dn, timeout=10):
	filename = f'{name}{dn}@2x.png'
	url = f'https://openweathermap.org/img/wn/{filename}'
	filepath = os.path.join(downloaddir, f'{filename}')
	if os.path.exists(filepath):
		return 'exists'

	if requests:
		try:
			r = requests.get(url, timeout=timeout, headers={'User-Agent': 'python-requests/2.x'})
			if r.status_code == 200 and r.content:
				with open(filepath, 'wb') as f:
					f.write(r.content)
				return 'downloaded'
			else:
				return f'HTTP {r.status_code}'
		except Exception as e:
			return f'error {e}'
	else:
		try:
			with urlrequest.urlopen(url, timeout=timeout) as resp:
				content = resp.read()
				if content:
					with open(filepath, 'wb') as f:
						f.write(content)
					return 'downloaded'
				else:
					return 'empty'
		except HTTPError as e:
			return f'HTTP {e.code}'
		except URLError as e:
			return f'URL error {e.reason}'
		except Exception as e:
			return f'error {e}'


def main(delay=0.1, timeout=10):
	os.makedirs(downloaddir, exist_ok=True)
	names = [ "01", "02", "03", "04", "09", "10", "11", "13", "50" ]
	dns = [ "d", "n" ]
	for name in names:
		for dn in dns:
			status = download_one(name, dn, timeout=timeout)
			print(f'{name}{dn}: {status}')
			time.sleep(delay)

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Download OpenWeatherMap png files numbered from START to END.')
	parser.add_argument('--delay', type=float, default=0.1, help='Delay in seconds between requests')
	parser.add_argument('--timeout', type=float, default=10, help='HTTP request timeout in seconds')
	args = parser.parse_args()
	try:
		main(args.delay, args.timeout)
	except KeyboardInterrupt:
		print('\nInterrupted by user', file=sys.stderr)
