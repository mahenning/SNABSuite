#!/usr/bin/env python
# -*- coding: utf-8 -*-

#   SNABSuite -- Spiking Neural Architecture Benchmark Suite
#   Copyright (C) 2017 Christoph Jenzen
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>


"""
Plots the membrane voltage, possibility to plot spike times on top of it
"""
import argparse

parser = argparse.ArgumentParser(description='Plot membrane potential graphs')

# Optional arguments
parser.add_argument("--ymin", type=float, help="minimal y-value")
parser.add_argument("--ymax", type=float, help="maximal y-value")
parser.add_argument("--xmin", type=float, help="minimal x-value")
parser.add_argument("--xmax", type=float, help="maximal x-value")
parser.add_argument("-t", type=int, help="Column of time-values", default=0)
parser.add_argument("-s", type=str, help="file containing spikes")
parser.add_argument(
    "-sc", type=int, help="Number of the Column containing spikes", default=0)


# Required Parameters
parser.add_argument("-y", type=int, required=True, help="Column of y-values")
parser.add_argument("file", metavar="file", type=str,
                    help="file with membrane voltage to plot")

args = parser.parse_args()


import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colorbar
import sys
import os

from dim_labels import *


def cm2inch(value):
    return value / 2.54


def get_max(data):
    idcs = np.isfinite(data)
    return np.max(data[np.isfinite(data)])


def get_min(data):
    idcs = np.isfinite(data)
    return np.min(data[np.isfinite(data)])


def plot_membrane(ax, xs, ys, color, ymin=None, ymax=None, xmin=None, xmax=None):
    """
    Plots membrane potential and spikes

    ax -- plt.figure object to plot into
    xs -- x values
    ys -- y values
    color -- color for the membrane potential graph
    ymin -- Minimal y value for limits of the plot
    ymax -- Maximal y value for limits of the plot
    xmin -- Mnimial x value for limits of the plot
    xmin -- Maximal x value for limits of the plot
    """

    ax.plot(xs, ys, color=color, lw=0.3, zorder=1, label="Membrane Voltage")

    ax.set_xlabel("Time in ms")
    ax.set_ylabel("Voltage in mV")

    # Set the limits of y-axxis
    if (ymin is None) and (ymax is None):
        ax.autoscale_view()
    elif ymax is None:
        ax.set_ylim(bottom=ymin)
    elif ymin is None:
        ax.set_ylim(top=ymax)
    else:
        ax.set_ylim(bottom=ymin, top=ymax)

    # Set the limits of x-axxis
    if (xmin is None) and (xmax is None):
        ax.set_xlim(left=get_min(xs), right=get_max(xs))
    elif xmax is None:
        ax.set_xlim(left=xmin, right=get_max(xs))
    elif xmin is None:
        ax.set_xlim(left=get_min(xs), right=xmax)
    else:
        ax.set_xlim(left=xmin, right=xmax)


fig = plt.figure(figsize=(cm2inch(12), cm2inch(6.0)))
ax = fig.add_subplot(111)

# Read membrane potential
results = np.genfromtxt(args.file, delimiter=',', names=None)
data = np.zeros((results.shape[0], len(results[0])))
for i in xrange(0, len(results)):
    data[i] = np.array(list(results[i]))

xs = np.array(data[:, args.t])
ys = np.array(data[:, args.y])

plot_membrane(ax, xs, ys, color="#000000", ymin=args.ymin,
              ymax=args.ymax, xmin=args.xmin, xmax=args.xmax)

# Plot spike times
if args.s is not None:
    results2 = np.genfromtxt(args.s, delimiter=',', names=None)
    # One dimensional data
    if not isinstance(results2[0], list):
        first = True
        for i in results2:
            if first:
                ax.axvline(x=i, linestyle=':', color='r',
                           lw=0.3, label="Spike Times")
                first = False
            else:
                ax.axvline(x=i, linestyle=':', color='r',
                           lw=0.3)
    else:
        # Convert data
        spikes = np.zeros((results2.shape[0], len(results2[0])))
        for i in xrange(0, len(results2)):
            spikes[i] = np.array(list(results2[i]))

        # Plot
        first = True
        for i in spikes[:, args.sc]:
            if first:
                ax.axvline(x=i, linestyle=':', color='r',
                           lw=0.3, label="Spike Times")
                first = False
            else:
                ax.axvline(x=i, linestyle=':', color='r',
                           lw=0.3)

# Write data to images folder
if not os.path.exists("images"):
    os.mkdir("images")
ax.legend(loc='lower center', bbox_to_anchor=(0.5, 1.05),
          ncol=4)
fig.savefig("images/" + args.file.split('/')[-1].split('.csv')
            [0] + ".pdf", format='pdf', bbox_inches='tight')
