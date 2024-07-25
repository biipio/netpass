"""Creates a copy of the inputted image(s) with a green overlay applied."""

import os
import sys
from PIL import Image, ImageOps


def main():
    filenames = sys.argv[1:]

    for filename in filenames:
        name, ext = os.path.splitext(filename)
        if ext != ".png":
            continue

        img = Image.open(filename)
        img = ImageOps.grayscale(img)
        img = ImageOps.colorize(img, black="#3fb170", white="#c5f6d9")
        img.save(f"{name}_preview.png", "PNG")


if __name__ == "__main__":
    main()
