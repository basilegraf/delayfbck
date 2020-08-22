#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat May 23 11:50:11 2020

@author: basile
"""

from oscpy.client import OSCClient
from time import sleep

osc = OSCClient(b'', 9001)
osc.encoding='utf8'



class delayfbck:
    def __init__(self, address, port=9001):
        self.address = address
        self.port = port
        self.osc = OSCClient(b'', port, encoding='utf8')
    
    def pluck(self, ampl=0.5, pos=0.2):
        self.osc.send_message(self.address, [u'pluck', ampl, pos])
    
    def delay(self, length, rampTime = 0.01, pitchCorrection = True):
        self.osc.send_message(self.address, [
            u'delay', length, rampTime, float(pitchCorrection)])
    
    def note(self,  noteNum, rampTime = 0.01, pitchCorrection = True):
        freq = (440 / 32) * (2 ** ((noteNum - 9) / 12))
        self.delay(1/freq, rampTime, pitchCorrection)
    
    def lp1(self, filtNum, freq, rampTime = 0.1):
        self.osc.send_message(self.address, [
            u'filter', filtNum, u'lp1', freq, rampTime])
    
    def hp1(self, filtNum, freq, rampTime = 0.1):
        self.osc.send_message(self.address, [
            u'filter', filtNum, u'hp1', freq, rampTime])
    
    def lp2(self, filtNum, freq, damp = 0.71, rampTime = 0.1):
        self.osc.send_message(self.address, [
            u'filter', filtNum, u'lp2', freq, damp, rampTime])
    
    def hp2(self, filtNum, freq, damp = 0.71, rampTime = 0.1):
        self.osc.send_message(self.address, [
            u'filter', filtNum, u'hp2', freq, damp, rampTime])
    
    def notch(self, filtNum, freq, gain, bandwidth, rampTime = 0.1):
        self.osc.send_message(self.address, [
            u'filter', filtNum, u'n', freq, gain, bandwidth, rampTime])
    
    def nonlin(self, gain, saturation, type = 'symmetric_sat'):    
        if (type != u'symmetric_sat') & (type != u'asymmetric_sat') &  (type != u'symmetric_sigmoid') & (type != u'asymmetric_sigmoid'):
            raise Exception('Unknown nonlinearity type')
        self.osc.send_message(self.address, [
            u'nonlin', type, gain, saturation])
        
    def ampctrl(self, ampl, propGain = 5.0, intGain = 20.0, saturation = 1.0, lpFreq = 5.0):
        self.osc.send_message(self.address, [
            u'ampctrl', lpFreq, ampl, propGain, intGain, saturation])
        
    def ampctrlOff(self):
        self.osc.send_message(self.address, [
            u'ampctrl', 5.0, 0.0, 0.0, 0.0, 0.0])
        
        
        
del1 = delayfbck(u'/delayfbck/del1')
del2 = delayfbck(u'/delayfbck/del2')
del3 = delayfbck(u'/delayfbck/del3')

del1.hp1(0, 10)
del2.hp1(0, 10)
del3.hp1(0, 10)

rho = 5
del1.lp1(1, rho*150)
del2.lp1(1, rho*200)
del3.lp1(1, rho*300)

sh = 0
del1.note(30+sh)
del2.note(37+sh)
del3.note(42+sh)

del1.delay(0.4)
del2.delay(0.2)
del3.delay(0.1)

del1.pluck(0.5,0)
del2.pluck(0.3,0)
del3.pluck(0.2,0)

del1.ampctrl(0.5)
del2.ampctrl(0.3)
del3.ampctrl(0.2)

del1.ampctrlOff()
del2.ampctrlOff()
del3.ampctrlOff()

