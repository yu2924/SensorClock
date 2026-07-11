from __future__ import annotations

from pathlib import Path

from PIL import Image


ROOT_DIR = Path(__file__).resolve().parent
INPUT_DIR = ROOT_DIR / "png"
OUTPUT_DIR = ROOT_DIR / "png_crop_72x72"
TARGET_SIZE = 72


def center_crop_72(img: Image.Image) -> Image.Image:
    width, height = img.size

    if width < TARGET_SIZE or height < TARGET_SIZE:
        scale = max(TARGET_SIZE / width, TARGET_SIZE / height)
        new_size = (int(round(width * scale)), int(round(height * scale)))
        img = img.resize(new_size, Image.LANCZOS)
        width, height = img.size

    left = (width - TARGET_SIZE) // 2
    top = (height - TARGET_SIZE) // 2
    right = left + TARGET_SIZE
    bottom = top + TARGET_SIZE

    return img.crop((left, top, right, bottom))


def main() -> None:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    for path in INPUT_DIR.glob("*.png"):
        with Image.open(path) as img:
            img = img.convert("RGBA")
            cropped = center_crop_72(img)
            out_path = OUTPUT_DIR / path.name
            cropped.save(out_path, format="PNG")


if __name__ == "__main__":
    main()
