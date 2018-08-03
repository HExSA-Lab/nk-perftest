#!/usr/bin/env python3
import csv
import collections
import numpy as np
import os

version = os.getenv('version', None)
if not version:
    raise RuntimeError('must pass a version=n ./plots.py')

skip_2d = bool(os.getenv('skip_2d', False))

x = os.getenv('x', None)
y = os.getenv('y', None)
if x is not None:
    x = float(x)
if y is not None:
    y = float(y)

series = {
    'nautk': {
        'file': f'data/nautk_{version}.csv',
        'label': 'nautk',
        'raw_data' : collections.defaultdict(lambda: collections.defaultdict(list)),
        'color': 'b',
    },
    'linux': {
        'file': f'data/linux_{version}.csv',
        'label': 'linux',
        'raw_data' : collections.defaultdict(lambda: collections.defaultdict(list)),
        'color': 'r',
    },
}
for os, seri in series.items():
    all_xy = []
    with open(seri['file']) as fil:
        reader = csv.DictReader(fil)
        for row in reader:
            x = int(row[reader.fieldnames[0]])
            y = int(row[reader.fieldnames[1]])
            all_xy.append((x, y))
            del row[reader.fieldnames[0]]
            del row[reader.fieldnames[1]]
            for key, val in row.items():
                if val:
                    seri['raw_data'][key][(x, y)].append(float(val))
    all_xy = np.array(all_xy)


# https://stackoverflow.com/a/11146645/1078199
def cartesian_product(arrays):
    la = len(arrays)
    dtype = np.find_common_type([a.dtype for a in arrays], [])
    arr = np.empty([len(a) for a in arrays] + [la], dtype=dtype)
    for i, a in enumerate(np.ix_(*arrays)):
        arr[..., i] = a
    return arr.reshape(-1, la)

def arg_nearest_2d(array, point):
    return ((array - point)**2).sum(axis=1).argmin()


import matplotlib.pyplot as plt
import warnings

def log10_nan(x, inplace=False):
    if not inplace:
        x = x.copy()
    mask = (~np.isnan(x)) & (x != 0)
    x[mask] = np.log10(x[mask])
    return x

unique_x = np.unique(all_xy[:, 0])
unique_y = np.unique(all_xy[:, 1])
grid_shape = unique_x.shape + unique_y.shape
xy_grid = cartesian_product([unique_x, unique_y])
for seri in series.values():
    seri['grid_data'] = {}
    for var in seri['raw_data'].keys():
        seri['grid_data'][var] = []
        for x, y in xy_grid:
            if (x, y) in seri['raw_data'][var]:
                val = np.mean(seri['raw_data'][var][(x, y)])
            else:
                val = np.nan
            seri['grid_data'][var].append(val)
        seri['grid_data'][var] = np.array(seri['grid_data'][var]).reshape(grid_shape)
xy_grid = xy_grid.reshape(grid_shape + (2,))

for seri in series.values():
    seri['raw_data_array'] = {}

for var in seri['raw_data'].keys():
    for seri in series.values():
        seri['raw_data_array'][var] = []
        for x, y in all_xy:
            seri['raw_data_array'][var].append(seri['raw_data'][var][(x, y)])
        seri['raw_data_array'][var] = np.array(seri['raw_data_array'][var])

if not skip_2d:
    kwargs = dict(cmap='RdBu', origin='lower', extent=[unique_y.min(), unique_y.max()+1, unique_x.min(), unique_x.max()+1])
    num_chunks = 10
    for var in seri['grid_data'].keys():
        if not var.startswith('sort'):
            print(f'skipping {var}')
            continue
        if 'dcache' in var:
            continue
        kwargs['cmap'] = 'RdBu'
        data = log10_nan(series['linux']['grid_data'][var]) - log10_nan(series['nautk']['grid_data'][var])
        extreme = np.max(np.fabs(data[~np.isnan(data)]))
        kwargs['vmin'] = -extreme
        kwargs['vmax'] = +extreme
        plt.imshow(data, **kwargs)
        colorbar = plt.colorbar()
        colorbar.set_label('blue means nautilus is faster (by $10^x$)')
        plt.ylabel(reader.fieldnames[0][2:])
        plt.xlabel(reader.fieldnames[1][2:])
        plt.title(f'{var}')
        plt.savefig(f'db_2d_{var}_comparison.png')
        plt.show()

        for seri in series.values():
            data2 = seri['grid_data'][var]
            if 'time' in var:
                print('dividing per element')
                data2 /= 2**(np.sum(xy_grid, axis=2) + num_chunks)
            else:
                print('log10')
                data2 = np.log10(data)

            kwargs['cmap'] = 'viridis'
            kwargs['vmin'] = data2.min()
            kwargs['vmax'] = data2.max()
            plt.imshow(data2, **kwargs)
            plt.title(f"{var} in {seri['label']}")
            colorbar = plt.colorbar()
            colorbar.set_label('(purple is better)')
            #plt.savefig(f"db_2d_{var}_{seri['label']}.png")
            plt.show()

        # kwargs['vmin'] = np.log10(kwargs['vmin'])
        # kwargs['vmax'] = np.log10(kwargs['vmax'])
        # for seri in series.values():
        #     plt.imshow(log10_nan(seri['grid_data'][var]), **kwargs)
        #     plt.title(f"{var} on {seri['label']} log")
        #     plt.show()


def aggregate(xs, ys, fn):
    unique_x = sorted(list(frozenset(map(tuple, xs))))
    result = []
    for x in unique_x:
        matching = (xs == x).all(axis=1)
        result.append(fn(ys[matching]))
    unique_x = np.array(unique_x)
    return unique_x, np.array(result)

from scipy.interpolate import interp2d

if x is not None:
    for var in seri['grid_data'].keys():
        if 'sort' not in var:
            continue
        # if 'miss' not in var:
        #     continue
        plt.figure(figsize=(9, 6))
        plt.title(f"{var} comparison @ x = {x}")
        series['nautk']['raw_data_array'].keys()
        for seri in series.values():
            label = seri['label']
            unique_xy, means = aggregate(all_xy, seri['raw_data_array'][var], np.mean)
            unique_xy, stds = aggregate(all_xy, seri['raw_data_array'][var], lambda x: np.std(x, ddof=1))
            mean_f = interp2d(unique_xy[:, 0], unique_xy[:, 1], means)
            std_f = interp2d(unique_xy[:, 0], unique_xy[:, 1], stds)
            zs = np.clip(mean_f(x, unique_y).flatten(), 0, None)
            zerr = np.clip(std_f(x, unique_y).flatten() * 5, 0, None)
            plt.plot(unique_y, zs, color=seri['color'])
            plt.fill_between(unique_y, zs - zerr, zs + zerr,
                             color=seri['color'], alpha=0.5, label=fr"{label}'s time $\pm$ 5 std. dev.")
        if 'time' in var:
            plt.ylabel('cycles')
        plt.legend()
        plt.xlabel(reader.fieldnames[1][2:])
        plt.show()


if x is not None and y is not None:
    pass
