import os
import sys
import ctypes
from PIL import Image

if sys.platform == "win32":
    sys.stdout.reconfigure(encoding="utf-8")
    kernel32 = ctypes.windll.kernel32
    handle = kernel32.GetStdHandle(-11)
    mode = ctypes.c_uint32()
    kernel32.GetConsoleMode(handle, ctypes.byref(mode))
    kernel32.SetConsoleMode(handle, mode.value | 0x0004)

RESET = "\033[0m"

SIZE_X = 80
SIZE_Y = 80

def output_path_for(img_path):
    stem = os.path.splitext(os.path.basename(img_path))[0]
    return os.path.join(os.path.dirname(__file__), "ascii", f"{stem}.h")


def image_to_ansi_rows(img_path):
    orig = Image.open(img_path)
    has_alpha = orig.mode == "RGBA"
    img = orig.convert("RGBA") if has_alpha else orig.convert("RGB")
    w, h = img.size
    aspect = h / w
    new_w = min(SIZE_X, int(SIZE_Y / aspect)) if aspect > 0 else SIZE_X
    new_h = int(new_w * aspect)
    if new_h > SIZE_Y:
        new_h = SIZE_Y
        new_w = int(new_h / aspect)
    if new_h % 2 != 0:
        new_h += 1
    img = img.resize((new_w, new_h), Image.LANCZOS)
    flat = img.tobytes()
    step = 4 if has_alpha else 3
    pixels = [flat[i:i + step] for i in range(0, len(flat), step)]
    rows_px = [pixels[i * new_w : (i + 1) * new_w] for i in range(new_h)]

    ALPHA_THRESH = 128
    ansi_rows = []
    for y in range(0, new_h, 2):
        row_chars = []
        for x in range(new_w):
            tp = rows_px[y][x]
            bp = rows_px[y + 1][x]
            tr, tg, tb = tp[0], tp[1], tp[2]
            br, bg, bb = bp[0], bp[1], bp[2]
            ta = tp[3] if has_alpha else 255
            ba = bp[3] if has_alpha else 255
            top_vis = ta >= ALPHA_THRESH
            bot_vis = ba >= ALPHA_THRESH
            if not top_vis and not bot_vis:
                row_chars.append(f" {RESET}")
            elif not top_vis:
                row_chars.append(f"\033[38;2;{br};{bg};{bb}m\u2584{RESET}")
            elif not bot_vis:
                row_chars.append(f"\033[38;2;{tr};{tg};{tb}m\u2580{RESET}")
            else:
                row_chars.append(
                    f"\033[38;2;{tr};{tg};{tb}m\033[48;2;{br};{bg};{bb}m\u2580{RESET}"
                )
        ansi_rows.append("".join(row_chars))

    return ansi_rows


def render_to_terminal(rows):
    print()
    for row in rows:
        print(row)
    print()


def save_to_header(rows, path, word):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write("#pragma once\n\n")
        f.write("namespace ui {\n")
        f.write("namespace ascii {\n\n")
        for i, row in enumerate(rows):
            escaped = row.replace("\\", "\\\\").replace('"', '\\"')
            escaped = escaped.replace("\x1b", "\\x1b")
            f.write(f'    const char* {word}_line{i} = "{escaped}";\n')
        f.write("\n")
        f.write(f"    const char* {word}_raw[] = {{\n")
        for i in range(len(rows)):
            f.write(f"        {word}_line{i},\n")
        f.write("    };\n\n")
        f.write(f"    constexpr int {word}_lineCount = {len(rows)};\n\n")
        f.write("} // namespace ascii\n")
        f.write("} // namespace ui\n")


if __name__ == "__main__":
    img_path = os.path.join(os.path.dirname(__file__), "png", f"{sys.argv[1] if len(sys.argv) > 1 else 'NA'}.png")
    out_path = output_path_for(img_path)
    print(f"Loading {img_path} ...")
    ansi_rows = image_to_ansi_rows(img_path)
    print(f"\nRendering image ({len(ansi_rows)} rows x {len(ansi_rows[0])} chars):")
    render_to_terminal(ansi_rows)
    save_to_header(ansi_rows, out_path, sys.argv[1] if len(sys.argv) > 1 else 'NA')
    print(f"[Done] Saved to {out_path}")
