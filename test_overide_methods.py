prev_visited_comb = []


def brute_force(k):
    hsh = f"{k[0]}{k[1]}{k[2]}"
    if not (hsh in prev_visited_comb):
        prev_visited_comb.append(hsh)
    else:
        print("Error visited same node 2 times", hsh)
        print('visited_nodes:', prev_visited_comb)
        exit(1)

    if len(prev_visited_comb) == 3**3:
        print('ALL NODES VISITED: passed')
        exit(0)


def check_byte(k, lst):
    return [1, 2, 30]
