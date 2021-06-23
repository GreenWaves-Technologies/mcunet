import argparse
import numpy as np


def create_parser():
    # create the top-level parser
    parser = argparse.ArgumentParser(prog='Read .npy')

    parser.add_argument('-npy', '--npy', required=True,
                        help="path to .npy file")
    return parser


def main():
    parser = create_parser()
    args = parser.parse_args()

    npy_file = args.npy
    data = np.load(npy_file, allow_pickle=True)
    print(data[0])
    print(data[0][0].shape)
    print(data[1])
    print(data[1][0].shape)
    output = data[-1][0]
    print(output)
    print("Predicted class:\t{}\nWith confidence:\t{}".format(np.argmax(output) + 1, output[np.argmax(output)]))

if __name__ == '__main__':
    main()
