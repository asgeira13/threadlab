#! /usr/bin/env python3
from colorama import Fore, Style, init
import os
import time
import sys
import argparse

init()
num_obj = 5
crossing = [0 for _ in range(num_obj)]
waiting = [0 for _ in range(num_obj)]
fairness = [0 for _ in range(num_obj)]
error = []
internal_order = [[] for _ in range(num_obj)]
over_K = 0
num_vehicle = 10
num_ped = 10
type_map = {
    0: "vertical pedestrian",
    1: "vertical vehicle",
    2: "horizontal pedestrian",
    3: "horizontal vehicle,"
}


frame_sleep = 0.1

threadmap = [{} for _ in range(num_obj)]
board = """\
          ---== Simple Crossing ==---

                     {vertWait}
================================================
                    |   |
                    |###|
                {horCarWait} |{vertCross}| {horCarCross}
                    |   |
                    |###|
                    |   |
================================================
                     {vertWait}
"""


def parse_line(fair, K):
    while 1:
        try:
            line = sys.stdin.readline()
        except:
            return

        if not line.strip():
            return
        words = line.split()
        if '[TL]:' in words[0]:
            if 'wait' in line:
                type, id, _,  queue_nr = words[2:]
                type = int(type)
                waiting[type] += 1
                threadmap[type][id] = int(queue_nr)
            elif 'cross' in line:
                type, command, _, id = words[3:]
                type = int(type)
                if 'start' in command:
                    crossing[type] += 1
                    waiting[type] -= 1
                    try:
                        queue_nr = threadmap[type].pop(id)
                        internal_order[type].append(queue_nr)
                    except:
                        error.append('Tried to remove thread' + id +
                                     'which does not exist')
                    legal_k_fairness(type, fair, K)
                elif 'end' in command:
                    crossing[type] -= 1
                yield True
        yield False


def legal_crash():
    vPed, vCar, hPed, hCar, _ = crossing
    if vPed > 0 and not hCar == 0:
        error.append('Vertical going pedestrian was run over')
    if hPed > 0 and not vCar == 0:
        error.append('Horizontal going pedestrian was run over')
    if hCar > 0 and not vCar == 0:
        error.append('There was a vehicle crash')


# external fairness
def legal_k_fairness(ind, fair, K):
    global over_K
    fairness[ind] = 0
    if ind == 0:    # vert_ped blocks horiz_vehicle
        fairness[3] += 1
    elif ind == 1:  # vert_vehicle blocks diag and horiz ped
        fairness[2] += 1
        fairness[4] += 1
    elif ind == 2:  # horiz_ped blocks vert_vehicle
        fairness[1] += 1
    elif ind == 3:  # vert_vehicle blocks horiz and diag ped
        fairness[0] += 1
        fairness[4] += 1
    elif ind == 4:  # diag_ped blocks vehicles
        fairness[1] += 1
        fairness[3] += 1

    for i in range(num_obj):
        if waiting[i] == 0:
            fairness[i] = 0

    if(fair):
        if not all(i <= K for i in fairness):
            error.append('K fairness not enforced: ' + str(fairness))
            for i in fairness:
                over_K = max(over_K, i/float(K))


def internal_fairness(K):
    for type in range(num_obj):
        exit_list = internal_order[type][::-1]
        for i, now in enumerate(exit_list):
            count = 0
            for before in exit_list[i:]:
                if before > now:
                    count += 1
            if count > K:
                error.append('Internal fairness not held '
                             'for thread of type %s who was %drd in line' %
                             (type_map[type], now))


def color(color, string):
    return color + str(string) + Style.RESET_ALL


def draw_ascii(sleeptime):
    os.system('clear')
    print(board.format(horCarWait=color(Fore.GREEN,
                                        "{:^3}".format(waiting[3])),
                       horCarCross=color(Fore.GREEN,
                                         "{:^3}".format(crossing[3])),
                       vertWait=color(Fore.RED,
                                      "{:^3}".format(waiting[0])),
                       vertCross=color(Fore.RED,
                                       "{:^3}".format(crossing[0]))))
    print("{:>35}".format("--=Fairness=--"))
    print("{:>35}".format("Vertical Pedestrians: %s" % color(Fore.RED,
                                                             fairness[0])))
    print("{:>35}".format("Horizontal Cars: %s" % color(Fore.GREEN,
                                                        fairness[3])))

    time.sleep(sleeptime)


def draw_step(sleeptime, fair, K):
    for frame in parse_line(fair, K):
        if not frame:
            continue
        legal_crash()
        draw_ascii(sleeptime)


def print_error():
    global over_K
    if not error:
        print('No errors detected')
    else:
        print('Errors:')
        for line in error:
            print(line)
        if over_K > 0:
            print('You went %d%% over fairness' % int((round(over_K-1,
                                                             2))*100))
    for i, type_d in enumerate(threadmap):
        if type_d != {}:
            for threadID in type_d:
                print("Thread %s of type %d did not exit" % (threadID, i))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Visualizes threadlab')
    parser.add_argument('time', type=float, default=0.1,
                        help='The duration between each drawn frame')
    parser.add_argument('-f', '--fairness', action='store_true',
                        default=False, help='Test fairness')
    args = parser.parse_args()
    arg_line = sys.stdin.readline()
    K, num_vehicle, num_ped = arg_line.split()
    K = int(K)
    draw_step(args.time, args.fairness, K)
    if(args.fairness):
        internal_fairness(K)
    print_error()
