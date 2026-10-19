import sys, shutil, os
sys.path.insert(0,'/tmp/claude-0/-home-claude/40adf7eb-52af-5f18-a6d3-0c8dd38ea351/scratchpad/ru')
from PIL import Image
from relabel import *
from tinyfont import F5, F4, F5N, F8
G='/tmp/claude-0/-home-claude/40adf7eb-52af-5f18-a6d3-0c8dd38ea351/scratchpad/bg/graphics/'
ORIG='/tmp/claude-0/-home-claude/40adf7eb-52af-5f18-a6d3-0c8dd38ea351/scratchpad/ru/orig_gfx/'
def load(name): return Image.open(ORIG+name)
def save(img,name): img.save(G+name, optimize=False)

def score_label(im):
    px=im.load(); pal=im.getpalette()
    # индексы берём из оригинала: белый, тень "#d", фон бара
    white=px[7,48]; shadow=px[8,49]; bg=px[10,47]
    assert min(pal[white*3:white*3+3])>=240, white
    for y in range(47,58):
        for x in range(7,25):
            if px[x,y] in (white,shadow): px[x,y]=bg
    rows,w=text_mask('ОЧКИ',F8,gap=1)
    x0=7
    for r,line in enumerate(rows):
        for c,ch in enumerate(line):
            if ch=='#': px[x0+c,48+r]=white
    for r,line in enumerate(rows):
        for c,ch in enumerate(line):
            if ch=='#' and (r+1>=len(rows) or rows[r+1][c]!='#') and px[x0+c,48+r+1]==bg: px[x0+c,48+r+1]=shadow

def panel(img):
    score_label(img)
    for (y0,x0,x1,y1),t in [((98,12,27,102),'РУКИ'),((98,40,63,102),'СБРОС'),((138,13,28,142),'АНТЕ'),((138,42,61,142),'РАУНД')]:
        relabel(img,(y0,x0,x1,y1),t,F4)

# ================= background_gfx =================
im=load('background_gfx.png'); panel(im)
relabel(im,(130,97,111,135),'ИГРАТЬ',F5)
relabel(im,(137,96,111,142),'РУКУ',F5)
relabel(im,(133,164,187,138),'СБРОС',F5)
relabel(im,(169,19,54,174),'МАЛЫЙ БЛАЙНД',F5)
relabel(im,(179,34,62,183),'ЦЕЛЬ',F4,align='right')
relabel(im,(209,26,54,214),'БИГ БЛАЙНД',F5)
relabel(im,(217,22,54,222),'БОСС БЛАЙНД',F5)
relabel(im,(234,2,30,238),'ЦЕЛЬ',F4,align='left')
relabel(im,(244,8,32,249),'ЗАНОВО',F5)
save(im,'background_gfx.png')

# ================= background_blind_select_gfx =================
im=load('background_blind_select_gfx.png'); panel(im)
names=[('МАЛЫЙ','БЛАЙНД'),('БИГ','БЛАЙНД'),('БОСС','БЛАЙНД')]
l1=[(170,84,100,175),(170,127,136,175),(170,165,178,175)]
l2=[(177,84,100,182),(177,124,140,182),(177,164,180,182)]
sel=[(162,83,101,167),(162,123,141,167),(162,163,181,167)]
sc =[(210,78,106,214),(210,118,146,214),(210,158,186,214)]
for p in range(3):
    relabel(im,sel[p],'ВЫБОР',F5,shadow=None,bg=15)
    relabel(im,sc[p],'ЦЕЛЬ',F4)
    i1=relabel(im,l1[p],'',F5N,defer=True,shadow=None)
    i2=relabel(im,l2[p],'',F5N,defer=True,shadow=None)
    cx=91+40*p
    #    widen(im,168,184,81+40*p,102+40*p,cx,3,3,6)
    c=91.5+40*p
    draw_text(im,i1,names[p][0],F5N,cx=c,shadow=False)
    draw_text(im,i2,names[p][1],F5N,cx=c,shadow=False)
relabel(im,(241,92,104,246),'ПАС',F5)
relabel(im,(241,132,144,246),'ПАС',F5)
# серые метки состояния (NEXT / SKIP / DEFEATED) и серые SKIP
gray=lambda p: p==20
relabel(im,(162,5,19,167),'ДАЛЕЕ',F5,white_fn=gray,bg=24,shadow=None)
relabel(im,(162,30,43,167),'ПАС',F5,white_fn=gray,bg=24,shadow=None)
relabel(im,(162,50,70,167),'ВЗЯТ',F5,white_fn=gray,bg=24,shadow=None)
relabel(im,(201,19,33,206),'ПАС',F5,white_fn=gray,bg=24,shadow=None)
relabel(im,(233,20,32,238),'ПАС',F5,white_fn=gray,bg=24,shadow=None)
save(im,'background_blind_select_gfx.png')

# ================= background_shop_gfx =================
im=load('background_shop_gfx.png'); panel(im)
relabel(im,(168,89,104,172),'ДАЛЕЕ',F4)
relabel(im,(174,87,106,178),'РАУНД',F4)
relabel(im,(187,86,109,191),'РЕРОЛЛ',F4)
save(im,'background_shop_gfx.png')

# ================= main menu =================
im=load('background_main_menu_gfx.png')
relabel(im,(126,96,110,131),'ИГРАТЬ',F5)
save(im,'background_main_menu_gfx.png')
print('done')

# ================= иконки блайндов (RGBA 32x32) =================
def chip(name, l1, l2):
    im = Image.open(ORIG+name+'.png').convert('RGBA'); px = im.load()
    W=(255,255,255,255); BODY=(71,71,71,255)
    for y in range(32):
        for x in range(32):
            if px[x,y]==W: px[x,y]=BODY
    for txt,y0 in ((l1,6),(l2,13)):
        rows,w=text_mask(txt,F5N)
        x0=int(round(11.5-w/2))
        for r,line in enumerate(rows):
            for c,ch in enumerate(line):
                if ch=='#': px[x0+c,y0+r]=W
    im.save(G+name+'.png')
chip('small_blind_gfx','МАЛ','БЛ.')
chip('big_blind_gfx','БИГ','БЛ.')
print('chips done')

# ================= вывеска SHOP -> ЛАВКА =================
BIG = {
'Л': ['..#####','..#####','..##.##','..##.##','..##.##','.##..##','.##..##','.##..##','##...##','##...##','##...##','##...##','##...##','##...##'],
'А': ['..###..','..###..','.##.##.','.##.##.','.##.##.','##...##','##...##','#######','#######','##...##','##...##','##...##','##...##','##...##'],
'В': ['#####.','#####.','##..##','##..##','##..##','##..##','#####.','#####.','##..##','##..##','##..##','##..##','#####.','#####.'],
'К': ['##...##','##...##','##..##.','##..##.','##.##..','##.##..','#####..','#####..','##.##..','##.##..','##..##.','##..##.','##...##','##...##'],
}
def shop_sign(word):
    im=Image.open(G+'background_shop_gfx.png'); px=im.load()
    for y in range(175,193):
        for x in range(15,58): px[x,y]=23
    gl=[BIG[c] for c in word]
    gap=2
    W=sum(len(g[0]) for g in gl)+gap*(len(gl)-1)
    x0=int(round((15+58)/2 - W/2))
    y0=177
    mask=set(); cx=x0
    for g in gl:
        for r,line in enumerate(g):
            for c,ch in enumerate(line):
                if ch=='#': mask.add((cx+c,y0+r))
        cx+=len(g[0])+gap
    for (x,y) in mask: px[x,y]=18
    for (x,y) in mask:
        if (x,y-1) not in mask or (x-1,y) not in mask: px[x,y]=12
    for (x,y) in mask:
        if (x,y+1) not in mask or (x+1,y) not in mask:
            px[x,y]= 11 if px[x,y]==18 else 15
    for (x,y) in mask:
        for dx,dy in ((1,1),(0,1),(1,0)):
            q=(x+dx,y+dy)
            if q not in mask and px[q]==23: px[q]=20
    im.save(G+'background_shop_gfx.png')
shop_sign('ЛАВКА')
