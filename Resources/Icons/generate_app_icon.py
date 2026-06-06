"""Generate NEXUS OSC Windows app icon source PNG."""

from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

SIZE = 512
BACKGROUND = (173, 255, 47, 255)  # #adff2f
FOREGROUND = (0, 0, 0, 255)
TEXT = "Nex"
MARGIN = 36

FONT_CANDIDATES = (
    Path(r"C:\Windows\Fonts\segoeuib.ttf"),
    Path(r"C:\Windows\Fonts\arialbd.ttf"),
    Path(r"C:\Windows\Fonts\calibrib.ttf"),
)


def load_font(size: int) -> ImageFont.FreeTypeFont | ImageFont.ImageFont:
    for path in FONT_CANDIDATES:
        if path.exists():
            return ImageFont.truetype(str(path), size)
    return ImageFont.load_default()


def fit_largest_font(draw: ImageDraw.ImageDraw, text: str, max_width: int, max_height: int):
    for size in range(420, 16, -2):
        font = load_font(size)
        bbox = draw.textbbox((0, 0), text, font=font)
        width = bbox[2] - bbox[0]
        height = bbox[3] - bbox[1]
        if width <= max_width and height <= max_height:
            return font, bbox

    font = load_font(16)
    return font, draw.textbbox((0, 0), text, font=font)


def main() -> None:
    image = Image.new("RGBA", (SIZE, SIZE), BACKGROUND)
    draw = ImageDraw.Draw(image)

    max_width = SIZE - MARGIN * 2
    max_height = SIZE - MARGIN * 2
    font, bbox = fit_largest_font(draw, TEXT, max_width, max_height)

    width = bbox[2] - bbox[0]
    height = bbox[3] - bbox[1]
    x = (SIZE - width) // 2 - bbox[0]
    y = (SIZE - height) // 2 - bbox[1]
    draw.text((x, y), TEXT, font=font, fill=FOREGROUND)

    output = Path(__file__).with_name("app_icon.png")
    image.save(output)
    print(f"Saved {output} ({output.stat().st_size} bytes, font size {font.size})")


if __name__ == "__main__":
    main()
