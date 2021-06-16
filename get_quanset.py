import os
import random
import argparse
import shutil
from PIL import Image

parser = argparse.ArgumentParser(description='Get random subtest from dataset')
parser.add_argument('-id', '--in_dir', type=str, help="Path to images directory")
parser.add_argument('-od', '--out_dir', type=str, help="Path to output directory")


if __name__ == '__main__':
    args = parser.parse_args()
    in_dir = args.in_dir
    out_dir = args.out_dir

    if not os.path.exists(in_dir):
        raise ValueError(f"Wrong path to images: {args.in_dir}")
    os.makedirs(out_dir, exist_ok=True)

    images_list = []
    for path, subdirs, files in os.walk(in_dir):
        for name in files:
            images_list.append(os.path.join(os.path.basename(path), name))
    random.seed(42)
    select_number = 1000
    print(len(images_list))
    print(select_number)
    randomly_chosen = random.sample(images_list, select_number)
    for file in randomly_chosen:
        shutil.copy2(os.path.join(in_dir, file), os.path.join(out_dir, os.path.basename(file)))

