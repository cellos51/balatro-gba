import sys
from collections import Counter
from PIL import Image
sys.path.insert(0, '/tmp/claude-0/-home-claude/40adf7eb-52af-5f18-a6d3-0c8dd38ea351/scratchpad/ru')
from tinyfont import F5, F4, text_mask

def is_white(pal, p):
    r, g, b = pal[p*3:p*3+3]
    return min(r, g, b) >= 240

def relabel(img, bbox, text, font, align='center', shadow='auto', anchor=None, dy=0, white_fn=None, bg=None, dx=0, defer=False, shadow_idx=None, white_idx=None):
    """bbox=(y0,x0,x1,y1) текущей надписи (белые пиксели). Стирает и рисует text."""
    px = img.load(); pal = img.getpalette()
    wf = white_fn or (lambda p: is_white(pal, p))
    y0, x0, x1, y1 = bbox
    T = {(x, y) for y in range(y0, y1) for x in range(x0, x1) if wf(px[x, y])}
    assert T, ('no text', bbox)
    white_idx = Counter(px[x, y] for x, y in T).most_common(1)[0][0] if white_idx is None else white_idx
    # теневые кандидаты
    below = Counter()
    for x, y in T:
        if (x, y+1) not in T and not wf(px[x, y+1]): below[px[x, y+1]] += 1
    cnt = Counter()
    for y in range(y0-1, y1+2):
        for x in range(x0-1, x1+1):
            if 0 <= x < img.width and 0 <= y < img.height and (x, y) not in T and not wf(px[x, y]): cnt[px[x, y]] += 1
    if bg is None:
        bg = cnt.most_common(1)[0][0]
    sh = None
    if shadow == 'auto':
        cands = [(c, n) for c, n in below.items() if c != bg]
        if cands:
            c, n = max(cands, key=lambda t: t[1])
            if n >= 0.15 * sum(below.values()): sh = c
    elif shadow not in (None, False):
        sh = shadow
    # стереть
    for x, y in T:
        px[x, y] = bg
    if sh is not None:
        for x, y in T:
            if px[x, y+1] == sh and (x, y+1) not in T: px[x, y+1] = bg
    info = dict(bg=bg, sh=sh, white=white_idx, bbox=bbox)
    if shadow_idx is not None: sh = shadow_idx
    info['sh'] = sh
    if defer: return info
    # нарисовать
    rows, w = text_mask(text, font)
    h = len(rows)
    ty = (y0 + dy)
    cx = (x0 + x1) / 2
    if align == 'center': tx = int(round(cx - w / 2)) + dx
    elif align == 'left': tx = x0 + dx
    elif align == 'right': tx = x1 - w + dx
    else: tx = anchor
    for r, line in enumerate(rows):
        for c, ch in enumerate(line):
            if ch == '#':
                px[tx+c, ty+r] = white_idx
    if sh is not None:
        for r, line in enumerate(rows):
            for c, ch in enumerate(line):
                if ch == '#' and (r+1 >= h or line_at(rows, r+1, c) != '#'):
                    if px[tx+c, ty+r+1] == bg: px[tx+c, ty+r+1] = sh
    return dict(bg=bg, sh=sh, white=white_idx, x=tx, y=ty, w=w, h=h)

def line_at(rows, r, c):
    return rows[r][c] if 0 <= r < len(rows) and 0 <= c < len(rows[r]) else '.'


def draw_text(img, info, text, font, cx=None, align='center', dx=0, dy=0, x0=None, shadow=True):
    px = img.load()
    y0, bx0, bx1, y1 = info['bbox']
    rows, w = text_mask(text, font)
    h = len(rows)
    if cx is None: cx = (bx0 + bx1) / 2
    if align == 'center': tx = int(round(cx - w / 2)) + dx
    elif align == 'left': tx = x0 + dx
    else: tx = int(x0 - w) + dx
    ty = y0 + dy
    for r, line in enumerate(rows):
        for c, ch in enumerate(line):
            if ch == '#': px[tx+c, ty+r] = info['white']
    sh = info['sh'] if shadow else None
    if sh is not None:
        for r, line in enumerate(rows):
            for c, ch in enumerate(line):
                if ch == '#' and line_at(rows, r+1, c) != '#' and px[tx+c, ty+r+1] == info['bg']:
                    px[tx+c, ty+r+1] = sh
    return tx, w

def widen(img, y0, y1, x0, x1, cx, add_left, add_right, fill_idx):
    """Расширяет блок [x0..x1] x [y0..y1): левая часть (<=cx) сдвигается влево, правая вправо,
    зазор заполняется столбцом cx. Область слева/справа предварительно закрашивается fill_idx."""
    px = img.load()
    cols = {x: [px[x, y] for y in range(y0, y1)] for x in range(x0, x1 + 1)}
    for x in range(x0 - add_left, x1 + add_right + 1):
        for y in range(y0, y1): px[x, y] = fill_idx
    for x in range(x0, cx + 1):
        for i, y in enumerate(range(y0, y1)): px[x - add_left, y] = cols[x][i]
    for x in range(cx + 1, x1 + 1):
        for i, y in enumerate(range(y0, y1)): px[x + add_right, y] = cols[x][i]
    for x in range(cx - add_left + 1, cx + 1 + add_right):
        for i, y in enumerate(range(y0, y1)): px[x, y] = cols[cx][i]
