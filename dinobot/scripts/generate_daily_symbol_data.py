# -*- coding: utf-8 -*-
# 
# file to generate data about the exchange everyday.
# the idea is we can then read this file by the C++ appliaction
# so it is easier to bootstrap.
#
# The idea being our c++ app can read this json file and get each exchanges 
# products and limits etc 
#
import os
import sys
import json
import time
import requests
import argparse

from datetime import datetime, timezone, timedelta

# gets absolute bin path
root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(root + '/external/ccxt/python')
import ccxt

import pprint
pp = pprint.PrettyPrinter(indent=4)

# ADD default exchnges we care about here. 
# here is a list of our default exchagnes we care about and want to generate data for
EXCHANGES = ['bitfinex', 'bitflyer', 'bitstamp', 'itbit', 'deribit',
            'kraken', 'bitmex', 'binance', 'coinbasepro'] 

# here we create a date string for the day in UTC time. the reason for the plus 10 minutes is
# when we will start this automatically for the trading day we will need to start the app
# before 00:00 UTC so the 'date' will be the day before. the reason only 10 minutes? 
# means if we run this ad hock the 10 mins won't really affect the date string except for the 
# 10 minutes before UTC, which is cool i think
def get_utc_date_string():
    utctime = datetime.now(timezone.utc) + timedelta(minutes=10)
    today = utctime.strftime('%Y%m%d')
    return today

#
# binance specific functions 
#
def binance_stream_data_gen(binance):
    res = {}
    wss = "wss://stream.binance.com:9443/stream?streams="
    sym_list = [x.replace("/", "").lower() for x in binance.symbols]

    res['symbol_list_slash'] = [x for x in binance.symbols]
    res['symbol_list'] = sym_list

    res['streams'] = {}

    trade_streams = ""
    ticker_streams = ""
    depth5_streams = ""
    kline5_streams = ""
    depth_streams = ""
    for s in sym_list:
        trade_streams += "/" + s + "@trade"
        ticker_streams += "/" + s + "@ticker"
        depth5_streams += "/" + s + "@depth5"
        kline5_streams += "/" + s + "@kline_5m"
        depth_streams += "/" + s + "@depth"
    trade_streams = trade_streams[1:]
    ticker_streams = ticker_streams[1:]
    depth5_streams = depth5_streams[1:]
    kline5_streams = kline5_streams[1:]
    depth_streams = depth_streams[1:]

    res['streams']['trade'] = wss  + trade_streams 
    res['streams']['ticker'] = wss  + ticker_streams
    res['streams']['depth5'] = wss  + depth5_streams 
    res['streams']['kline5'] = wss  + kline5_streams
    res['streams']['depth'] = wss  + depth_streams 
    
    return res

#
# Coinbase pro
#
def coinbasepro_data_gen(coinbase):
    res = {}
    res['symbol_list_dash'] = sorted([x.replace("/", "-")  for x in coinbase.symbols])
    res['symbol_list'] = sorted([x.replace("/", "").lower() for x in coinbase.symbols])

    return res

#
# Bitfinex 
#
def bitfinex_data_gen(bitfinexexch):
    res = {}

    bitfinex = bitfinexexch.load_markets()
    prods = []
    currencies = set()
    
    for k, v in bitfinex.items():
        if v['active']:
            prods.append(v['id'])
            currencies.add(v['quoteId'])
    
    res['symbol_list'] = sorted(prods)
    res['currencies'] = sorted(list(currencies))

    return res
    
#
# BitMEX
#
def bitmex_data_gen(bitmexexch):
    res = {}

    bitmex = bitmexexch.load_markets()
    prods = []
    
    for k, v in bitmex.items():
        if v['active']:
            prods.append(v['id'])

    res['symbol_list'] = sorted(prods)

    return res

#
# Kraken
#
def kraken_data_gen(krakenexch):
    res = {}

    kraken = krakenexch.load_markets()
    prods = []
    
    for k, v in kraken.items():
        if v['active']:
            prods.append(v['id'])

    res['symbol_list'] = sorted(prods)
    return res

#
# Deribit
#
def deribit_data_gen(deribitexch):
    res = {}
    
    deribit = deribitexch.load_markets()
    prods = []
    
    for k, v in deribit.items():
        if v['active']:
            prods.append(v['id'])

    res['symbol_list'] = sorted(prods)
    
    return res

#
# Bitstamp
#
def bitstamp_data_gen(bitstampexch):
    res = {}

    bitstamp = bitstampexch.load_markets()
    prods = []
    
    for k, v in bitstamp.items():
        if v['active']:
            prods.append(v['id'])

    res['symbol_list'] = sorted(prods)
    
    return res

def main():
    global EXCHANGES # pretty shit to use this but it works and it's only a small script...

    parser = argparse.ArgumentParser(description='application which requests and prints out daily exchange data for use in trading application')
    parser.add_argument('-o','--output_dir', dest='outdir' , help='directory we will output all the exchagne details', required=True, type=str)
    parser.add_argument('-e','--exchanges', dest='exchanges', help='list of exchanges, if none supplied assumes all exchanges wanted', required=False, type=str)
    args = vars(parser.parse_args())

    output_base = args['outdir'] + '/' + 'dinobot.'
    if args['exchanges']:
        EXCHANGES = args['exchanges'].split(',')

    for e in EXCHANGES:
        outputfile = output_base + 'startup.' + e + '.' + get_utc_date_string() + '.json'
        glog = open(outputfile, 'w')
        
        exch = getattr (ccxt, e) ()
        
        res = {}
        res['data_type'] = 'markets'
        res['exchange'] = e
        res['exchange_details'] = exch.load_markets()
        res['exchagne_currencies'] =  {x:y for (x,y) in exch.currencies.items()} 

        #
        # extra stuff for certain exchanges 
        #

        # binance has a weird websocket thing where we need to have a list of 
        # lots of different streams per websocket conenction endpoint
        if e in 'binance':
            res['binance'] = binance_stream_data_gen(exch)
        elif e in 'coinbasepro':
            res['coinbasepro'] = coinbasepro_data_gen(exch)
        elif e in 'bitmex':
            res['bitmex'] = bitmex_data_gen(exch)
        elif e in 'bitfinex':
            res['bitfinex'] = bitfinex_data_gen(exch)
        elif e in 'deribit':
            res['deribit'] = deribit_data_gen(exch)
        elif e in 'kraken':
            res['kraken'] = kraken_data_gen(exch)
        elif e in 'bitstamp':
            res['bitstamp'] = bitstamp_data_gen(exch)




        # print out the full JSON object
        #pp.pprint(res))
        glog.write(json.dumps(res) + '\n')

        glog.close()

        print('downloading ' + e + ' to ' + outputfile)

if __name__ == '__main__':
    main()
