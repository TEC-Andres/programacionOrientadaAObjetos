import os
import sys
import io
from PIL import Image, ImageFont, ImageDraw

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

FONT_PATH = os.path.join(os.path.dirname(__file__), "ttf/5x5.ttf")
FONT_SIZE = 10
BG_SHADE = "\033[48;2;34;34;34m"
BG_NONE = "\033[49m"
COLOR_GRAY = "\033[38;5;244m"
COLOR_WHITE = "\033[38;5;255m"
RESET = "\033[0m"

def render_text_to_matrix(text, font_path, font_size):
    if not os.path.exists(font_path):
        raise FileNotFoundError(f"Could not find the font file at: {font_path}")
    try:
        font = ImageFont.truetype(font_path, font_size)
    except IOError:
        font = ImageFont.load_default()

    canvas_w = font_size * len(text) * 2
    canvas_h = font_size * 3
    img = Image.new("1", (canvas_w, canvas_h), 0)
    draw = ImageDraw.Draw(img)
    draw.text((0, 0), text, font=font, fill=1)

    bbox = img.getbbox()
    if not bbox:
        return [[0]], 0

    img = img.crop((0, 0, bbox[2], bbox[3]))
    width, height = img.size
    pixels = list(img.getdata())
    matrix = [pixels[i * width:(i + 1) * width] for i in range(height)]
    current_x = 0
    for char in text:
        current_x += draw.textlength(char, font=font)

    midpoint_boundary = current_x / 2
    return matrix, midpoint_boundary

def generate_ansi_art(text, trunk_width=0, trunk_height=0):
    text = text.lower()
    matrix, split_x = render_text_to_matrix(text, FONT_PATH, FONT_SIZE)
    height = len(matrix)
    width = len(matrix[0])
    shade_start_row = int(height * 0.80)
    shade_end_row = int(height * 1.00)
    ansi_rows = []

    max_rows = height // 2
    if trunk_height > 0 and trunk_height < max_rows:
        max_rows = trunk_height

    for y in range(0, height, 2):
        if len(ansi_rows) >= max_rows:
            break
        row_output = ""
        max_cols = width
        if trunk_width > 0 and trunk_width < max_cols:
            max_cols = trunk_width
        for x in range(max_cols):
            top_pixel = matrix[y][x] if y < height else 0
            bot_pixel = matrix[y+1][x] if (y+1) < height else 0
            use_bg_top = (shade_start_row <= y <= shade_end_row)
            use_bg_bot = (shade_start_row <= (y + 1) <= shade_end_row)

            color = COLOR_GRAY if x < split_x else COLOR_WHITE
            in_shade = use_bg_top or use_bg_bot
            bg = BG_SHADE if in_shade else BG_NONE
            if top_pixel and bot_pixel:
                row_output += f"{bg}{color}\u2588{RESET}"
            elif top_pixel and not bot_pixel:
                row_output += f"{bg}{color}\u2580{RESET}"
            elif not top_pixel and bot_pixel:
                row_output += f"{bg}{color}\u2584{RESET}"
            else:
                row_output += f"{bg} {RESET}"
        print(row_output)
        ansi_rows.append(row_output)
    return ansi_rows

def generate_cpp_header(word, ansi_rows):
    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("namespace ui {")
    lines.append("namespace ascii {")
    lines.append("")

    for i, row in enumerate(ansi_rows):
        escaped = row.replace("\\", "\\\\").replace('"', '\\"')
        lines.append(f'    const char* {word}_line{i:d} = "{escaped}";')

    lines.append("")
    lines.append(f"    const char* {word}_raw[] = {{")
    for i in range(len(ansi_rows)):
        comma = "," if i < len(ansi_rows) - 1 else ""
        lines.append(f"        {word}_line{i:d}{comma}")
    lines.append("    };")
    lines.append("")
    lines.append(f"    constexpr int {word}_lineCount = {len(ansi_rows)};")
    lines.append("")
    lines.append("} // namespace ascii")
    lines.append("} // namespace ui")
    return "\n".join(lines) + "\n"

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Generate ANSI art header from text")
    parser.add_argument("word", nargs="?", default="movieverse",
                        help="Word to render as ANSI art")
    parser.add_argument("--width", type=int, default=0,
                        help="Truncate output to WIDTH columns")
    parser.add_argument("--height", type=int, default=0,
                        help="Truncate output to HEIGHT rows")
    parser.add_argument("--no-save", action="store_true",
                        help="Only print to console, don't write header file")
    args = parser.parse_args()

    print(f"\nRendering '{args.word}' with accurate background masking:\n")
    rows = generate_ansi_art(args.word, args.width, args.height)

    if not args.no_save:
        header = generate_cpp_header(args.word, rows)
        out_dir = os.path.dirname(__file__)
        filename = f"{args.word}.h".lower()
        out_path = os.path.join(out_dir, filename)
        with open(out_path, "w", encoding="utf-8") as f:
            f.write(header)
        print(f"\n[Success] Header saved to {out_path}")

        mapping_path = os.path.join(out_dir, "__mapping.h")
        if os.path.exists(mapping_path):
            include_line = f'#include "{filename}"'
            with open(mapping_path, "r", encoding="utf-8") as f:
                content = f.read()
            if include_line not in content:
                marker = "// #include \"other_art.h\""
                if marker in content:
                    content = content.replace(marker, f"{include_line}\n{marker}")
                    with open(mapping_path, "w", encoding="utf-8") as f:
                        f.write(content)
                    print(f"[Success] Registered in __mapping.h")
