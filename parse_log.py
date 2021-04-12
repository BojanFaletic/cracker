import re
import numpy as np
import matplotlib.pyplot as plt


def main(f_name='arduino/putty_output.log'):
    file = None
    try:
        file = open(f_name, 'r')
    except FileNotFoundError:
        print("Unable to find putty log file")
        return -1

    # create dictionary of all digits
    digit_ms = {}
    digit_ticks = {}
    for i in range(256):
        digit_ms[i] = []
        digit_ticks[i] = []

    # Tracking items in current batch
    is_batch_valid = False
    line_counter = 0
    batch_idx = 0
    incomplete_batch_ms = {}
    incomplete_batch_ticks = {}

    while line := file.readline():
        line_counter += 1
        if 'Sending' in line:
            # template = 'Sending: 14 required time: 30.17 ms  1475989'
            digit = int(re.findall('Sending: (\d*)', line)[0])
            ms_time = float(re.findall('(\d*\.\d*) ms', line)[0])
            tick_time = float(re.findall('  (\d*)', line)[0])

            # Start of batch
            if digit == 0:
                batch_idx = 0
                is_batch_valid = True
                incomplete_batch_ms.clear()
                incomplete_batch_ticks.clear()

            # Save times to array
            incomplete_batch_ms[digit] = ms_time
            incomplete_batch_ticks[digit] = tick_time

            if not is_batch_valid:
                continue

            # Drop batch because data is incomplete
            if (digit != batch_idx):
                print(f'''Discarding batch: {digit} != {batch_idx}, \
                    line number: {line_counter}''')
                is_batch_valid = False
                continue
            batch_idx += 1

            # Batch is finished add to list of samples
            if (digit == 255):
                for i in range(256):
                    digit_ms[i].append(incomplete_batch_ms[i])
                    digit_ticks[i].append(incomplete_batch_ticks[i])

    # Process data
    process_dict = {}
    for i in range(256):
        min_ms = np.min(digit_ms[i])
        max_ms = np.max(digit_ms[i])
        average_ms = np.mean(digit_ms[i])
        std = np.std(digit_ms[i])

        process_dict[i] = [min_ms, max_ms, average_ms, std]

    # Find candate of each passing
    score_board_max = np.zeros(256)
    score_board_min = np.zeros(256)
    for batch in range(len(digit_ms[255])):
        std = np.std([digit_ms[i][batch] for i in range(256)])
        average = np.mean([digit_ms[i][batch] for i in range(256)])

        min_th = average - std
        max_th = average + std
        for i in range(256):
            score_board_max[i] += 1 if digit_ms[i][batch] > max_th else 0
            score_board_min[i] += 1 if digit_ms[i][batch] < min_th else 0

        # Print summary
        items_in_batch_ticks = [digit_ticks[i][batch] for i in range(256)]
        max_digit = np.argmax(items_in_batch_ticks)
        max_digit_ms = digit_ms[max_digit][batch]

        print(f'''Deviation per batch {batch}: {std:.4f}, \
            max digit: {max_digit}, with value: {max_digit_ms} ms''')

    # Plot results
    t = np.arange(256)
    plt.plot(t, score_board_max, 'r')
    plt.plot(t, score_board_min, 'b')
    plt.xlabel('Digit [n]')
    plt.ylabel('Pass threshold [n]')
    plt.legend(['Max'], ['Min'])
    plt.show()

    # Plot data
    t = np.arange(256)
    min_data = np.array([process_dict[i][0] for i in range(256)])
    max_data = np.array([process_dict[i][1] for i in range(256)])
    average_data = np.array([process_dict[i][2] for i in range(256)])
    std_data = np.array([process_dict[i][3] for i in range(256)])

    bottom_interval = average_data - 1*std_data
    upper_interval = average_data + 1*std_data

    '''
    # Divination
    plt.plot(t, std_data)
    plt.xlabel('Digits [n]')
    plt.ylabel('Divination [ms]')
    plt.show()
    '''

    # Summary
    plt.plot(t, min_data, t, max_data, t, average_data)
    plt.plot(t, bottom_interval, 'c-')
    plt.plot(t, upper_interval, 'c-')
    plt.xlabel('Digits [n]')
    plt.ylabel('Time [ms]')
    plt.show()


if __name__ == "__main__":
    main()
