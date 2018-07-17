#!/usr/bin/env python3

import csv
import matplotlib.pyplot as plt
import numpy as np
import re
import os

version = os.getenv('version', 4)

only_plots = [tuple(map(int, pair.split(',')))
              for pair in os.getenv('only_plots', '').split(':')
              if pair]

data_file_names = {'linux': f'data/linux{version}.csv', 'nautk': f'data/nautk{version}.csv'}

plots = {}
headers = {}

def is_int(x):
    try:
        int(x)
    except ValueError:
        return False
    else:
        return True

for this_os, data_file_name in data_file_names.items():
    title = None
    with open(data_file_name) as data_file:
        this_data = csv.reader(data_file)
        for row in this_data:
            if row[-1] == 'title row':
                title = row[-2]
                header_row = row[:-2]
                if title not in plots:
                    plots[title] = {}
                plots[title][this_os] = [[] for col in header_row]
                headers[title] = header_row
            else:
                for col_no, data in enumerate(row):
                    try:
                        data = int(data)
                    except ValueError as e:
                        print(this_os)
                        print(header_row)
                        print(row)
                        raise e
                    plots[title][this_os][col_no].append(data)

for title in plots.keys():
    for this_os in plots[title].keys():
        for col_no in range(len(plots[title][this_os])):
            plots[title][this_os][col_no] = np.array(plots[title][this_os][col_no])

def deduplicate(xs, ys):
    unique_xs = sorted(list(set(xs)))
    means = []
    stds = []
    for x in unique_xs:
        means.append(ys[xs == x].mean())
        stds.append(ys[xs == x].std())
    return np.array(unique_xs), np.array(means), np.array(stds)

colors = {'linux': 'r', 'nautk': 'b'}

for plot_no, plot_title in enumerate(sorted(plots.keys())):
    header_row = headers[plot_title]
    n_columns = len(list(plots[plot_title].values())[0])

    for col_no in range(n_columns):
        if only_plots and (plot_no, col_no) not in only_plots:
            continue
        if col_no == 0:
            continue

        plt.figure(figsize=(9, 6))
        col_title = header_row[col_no]
        if col_title:
            plt.title(f'{plot_title} {col_title} ({version}:{plot_no}:{col_no})')
        else:
            plt.title(f'{plot_title} ({version}:{plot_no}:{col_no})')

        plt.xlabel(header_row[0])

        for this_os in plots[plot_title].keys():

            xs = plots[plot_title][this_os][0]
            ys = plots[plot_title][this_os][col_no]

            xs, means, stds = deduplicate(xs, ys)
            if this_os == 'linux':
                base_means = means
            else:
                factors = means / base_means
                for x, factor, mean in list(zip(xs, factors, means))[::2]:
                    plt.text(x, mean, s=f'{factor:.1f}x')

            plt.plot(xs, means, color=colors[this_os])
            plt.fill_between(xs,
                             np.clip(means - stds, 1, None),
                             np.clip(means + stds, 1, None),
                             color=colors[this_os], alpha=0.5, label=fr"{this_os}'s time $\pm$ std. dev.")

        plt.legend(loc='upper left')
        plt.ylabel('time (cycles)')
        plt.show()
        #plt.savefig(f'/home/sam/reu/pres4/reveal.js/img/{plot_title}_{col_title}.png', dpi=150, bbox_inches='tight')
