import os
import sys
import ctypes
import pyperclip
from PIL import Image, ImageFont, ImageDraw

if sys.platform == "win32":
    kernel32 = ctypes.windll.kernel32
    handle = kernel32.GetStdHandle(-11)
    mode = ctypes.c_uint32()
    kernel32.GetConsoleMode(handle, ctypes.byref(mode))
    kernel32.SetConsoleMode(handle, mode.value | 0x0004)

FONT_PATH = os.path.join(os.path.dirname(__file__), "ttf/5x5.ttf")
FONT_SIZE = 10
BG_SHADE = "\033[48;2;34;34;34m"  # #222222 Dark Gray background strip (24-bit true color)
BG_NONE = "\033[49m"           # Resets background to default terminal color
COLOR_GRAY = "\033[38;5;244m"  # Dim/Grayed out foreground text (left half)
COLOR_WHITE = "\033[38;5;255m" # Bright White foreground text (right half)
RESET = "\033[0m"              # Reset all formatting

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

def generate_ansi_art(text):
    text = text.lower()
    matrix, split_x = render_text_to_matrix(text, FONT_PATH, FONT_SIZE)
    height = len(matrix)
    width = len(matrix[0])
    shade_start_row = int(height * 0.80)
    shade_end_row = int(height * 1.00)
    ansi_rows = []
    
    for y in range(0, height, 2):
        row_output = ""
        for x in range(width):
            top_pixel = matrix[y][x] if y < height else 0
            bot_pixel = matrix[y+1][x] if (y+1) < height else 0
            use_bg_top = (shade_start_row <= y <= shade_end_row)
            use_bg_bot = (shade_start_row <= (y + 1) <= shade_end_row)
            
            color = COLOR_GRAY if x < split_x else COLOR_WHITE
            in_shade = use_bg_top or use_bg_bot
            bg = BG_SHADE if in_shade else BG_NONE
            if top_pixel and bot_pixel:
                row_output += f"{bg}{color}█{RESET}"
            elif top_pixel and not bot_pixel:
                row_output += f"{bg}{color}▀{RESET}"
            elif not top_pixel and bot_pixel:
                row_output += f"{bg}{color}▄{RESET}"
            else:
                row_output += f"{bg} {RESET}"
        print(row_output)
        ansi_rows.append(row_output)
    return "\n".join(ansi_rows)

if __name__ == "__main__":
    word = "movieverse"
    print(f"\nRendering '{word}' with accurate background masking:\n")    
    logo = generate_ansi_art(word)    
    pyperclip.copy(logo)
    print("\n[Success] ANSI art copied to clipboard!")