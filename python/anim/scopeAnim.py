#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Sep 27 11:32:50 2020

@author: basile
"""

import sympy as sp
import numpy as np
import scipy.integrate as integrate
import scipy.optimize as opt
import matplotlib.pyplot as plt
from scipy.io import wavfile

import matplotlib.animation as animation

from matplotlib.animation import FuncAnimation


# Use qt for animations
try:
    import IPython
    shell = IPython.get_ipython()
    shell.enable_matplotlib(gui='qt')
except:
    pass

saveAnimation = True

plt.close('all')

samplerate, data = wavfile.read('/home/basile/Music/delayfbck/slow_factory.wav')

fps = 32
rate = 44100
nPerFrame = 5693

x = data.astype('float')
x = x[:,0]+x[:,1]

#x = x[10000000:12000000]

class animScope:
    def __init__(self, data, npf, fps, sr):
        
        self.data = (data-data.mean()) / (data.max() - data.min())
        self.npf = npf;
        self.fps = fps;
        self.sr = sr;
        self.time = 0;
        nFrames = int(fps * (len(self.data)-self.npf) / sr)
        self.frames = range(0, nFrames)
        self.fig, self.ax = plt.subplots()
        
        self.ax.clear()
        self.ln, = self.ax.plot([], [],linewidth=6)
        self.ax.set_aspect(aspect='auto', adjustable='box')
        self.ax.set_xlim(left=0, right=self.npf)
        self.ax.set_ylim(bottom=-0.52, top=0.52)
        self.ax.set_xbound(lower=0, upper=self.npf)
        self.ax.set_ybound(lower=-0.52, upper=0.52)
        self.ax.grid(b=False)
        self.ax.axis('off')
        self.fig.patch.set_facecolor((0,0,0))
        self.fig.patch.set
        self.ax.set_facecolor((0,0,0))
        
        self.fig.set_size_inches(19.20, 10.80, True)
        self.fig.set_dpi(100)
        self.fig.tight_layout()
    def initAnim(self):
        print('meuh')
        

    def update(self, frame):
        n = len(self.data) - self.npf
        idx = int(np.floor(frame/len(self.frames) * n))
        self.ln.set_xdata(range(0,self.npf))
        self.ln.set_ydata(self.data[idx : idx+self.npf])
       
    def anim(self):
        return FuncAnimation(self.fig, self.update, self.frames, init_func=self.initAnim, blit=False, repeat=False, interval=1000/self.fps)
        

if True:
    scope = animScope(x, nPerFrame, fps, rate)
    aa = scope.anim()
    writer = animation.FFMpegWriter(fps=fps, metadata=dict(artist='Ugarte'), bitrate=3000)
    aa.save('SlowFactory.mp4', writer=writer,dpi=100, savefig_kwargs=dict(facecolor=(0,0,0)))

