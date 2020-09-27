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

x = np.linspace(0,1000,50000);
y = np.cos(x) * x


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
        self.fig.tight_layout()
        self.ax.clear()
        self.ln, = self.ax.plot([], [])
        self.ax.set_aspect(aspect='auto', adjustable='box')
        self.ax.set_xlim(left=0, right=self.npf)
        self.ax.set_ylim(bottom=-0.52, top=0.52)
        self.ax.set_xbound(lower=0, upper=self.npf)
        self.ax.set_ybound(lower=-0.52, upper=0.52)
        self.ax.grid(b=False)
        self.ax.axis('off')
        self.fig.patch.set_facecolor('xkcd:black')
        self.ax.set_facecolor((0,0,0))
        
        self.fig.set_size_inches(19.20, 10.80, True)
        self.fig.set_dpi(100)
    def initAnim(self):
        print('meuh')
        

    def update(self, frame):
        print(frame)
        n = len(self.data) - self.npf
        idx = int(np.floor(frame/len(self.frames) * n))
        self.ln.set_xdata(range(0,self.npf))
        self.ln.set_ydata(self.data[idx : idx+self.npf])
        print(idx, idx+self.npf)
       
    def anim(self):
        return FuncAnimation(self.fig, self.update, self.frames, init_func=self.initAnim, blit=False, repeat=False, interval=1000/self.fps)
        

scope = animScope(y, 300, 25, 44100)
aa = scope.anim()

#if saveAnimation:
#    aa.save('beforeOptim.gif', writer='imagemagick', fps=25)

#Writer = animation.writers['ffmpeg']
#writer = Writer(fps=25, metadata=dict(artist='Ugarte'), bitrate=1000)

writer = animation.FFMpegWriter(fps=25, codec=None, bitrate=3000 , metadata=None)

aa.save('HeroinOverdosesJumpy.mp4', writer=writer,dpi=100)

