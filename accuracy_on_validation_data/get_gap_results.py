import os
import argparse
from PIL import Image
import shutil
from tqdm import tqdm

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
    print(len(images_list))
    for file in tqdm(images_list):
        shutil.copy2(os.path.join(in_dir, file), "image.ppm")
        csv_name = file.replace("/", "_")
        csv_name = csv_name.replace(".ppm", ".csv")
        print(csv_name)
        run_string = "gapy --target=gapuino --platform=gvsoc \
            --work-dir=/media/vkoriukina/2048affa-7307-45b5-babc-21ccb6136387/vkoriuki/Repos/mcunet/BUILD/GAP8/GCC_RISCV \
            --config-opt=**/flash/content/partitions/readfs/files=/media/vkoriukina/2048affa-7307-45b5-babc-21ccb6136387/vkoriuki/Repos/mcunet/model_L3_Flash_Const.dat \
            --config-opt=flash/content/partitions/readfs/files=/media/vkoriukina/2048affa-7307-45b5-babc-21ccb6136387/vkoriuki/Repos/mcunet/model_L3_Flash_Const.dat \
            run --exec-prepare --exec --binary=/media/vkoriukina/2048affa-7307-45b5-babc-21ccb6136387/vkoriuki/Repos/mcunet/BUILD/GAP8/GCC_RISCV/test"

        os.system(run_string)
        shutil.copy2("gap_result.csv", os.path.join(out_dir, csv_name))


