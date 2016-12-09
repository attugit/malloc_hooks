#!/usr/bin/env python3

import re
import sys
import json
import weakref
import argparse

frame = re.compile(
    '.*malloc_hook <pointer=(0x\w+)> <size=(\d+)> <frame=(\d+)> <symbol=(.*)>.*')
free = re.compile('.*malloc_hook freed pointer <free=(0x\w+)>.*')

parser = argparse.ArgumentParser()
parser.add_argument('-i', '--input', type=argparse.FileType('r'))
parser.add_argument('-o', '--output', type=argparse.FileType('w'))
parser.add_argument('-n', '--number', type=int, default=-1)
parser.add_argument(
    '-s',
    '--sort-by',
    dest='sortby',
    choices=[
        'atexit',
        'count',
        'sum',
        'symbol',
        'peak',
        'max'],
    default='peak')
group = parser.add_mutually_exclusive_group()
group.add_argument('-a', '--ascending', action='store_true')
group.add_argument('-d', '--descending', action='store_false')

args = parser.parse_args()


class proxy_list(list):
    pass


def partial_sum(iterable):
    total = 0
    for x in iterable:
        total += x
        yield total


def handleMalloc(symbols, allocs, ptr, size, idx, symbol):
    size = int(size)
    symbols.setdefault(symbol, proxy_list()).append(size)
    allocs.setdefault(
        ptr, (size, []))[1].append(
        weakref.proxy(symbols[symbol]))


def handleFree(allocs, ptr):
    try:
        size, proxies = allocs[ptr]
        for p in proxies:
            p.append(-size)
    except KeyError as e:
        pass


def parse(lines):
    symbols = {}
    allocs = {}
    for ex in lines:
        result = frame.match(ex)
        if result:
            handleMalloc(symbols, allocs, *result.groups())
            continue
        result = free.match(ex)
        if result:
            handleFree(allocs, *result.groups())
        return symbols


def analyze(symbols):
    ret = []
    for sym, allocs in symbols.items():
        partial = tuple(partial_sum(allocs))
        ret.append({'symbol': sym,
                    'max': max(allocs),
                    'atexit': partial[-1],
                    'peak': max(partial),
                    'sum': sum(allocs),
                    'count': len(allocs)})
    return ret

indata = args.input if args.input is not None else sys.stdin
output = args.output if args.output is not None else sys.stdout
direction = False if args.ascending else True

symbols = parse(indata.readlines())
items = analyze(symbols)
items.sort(key=lambda x: x[args.sortby], reverse=direction)
output.write(json.dumps(items[:args.number], sort_keys=True, indent=2) + '\n')
