"""Review partial assemblies at native texel, 1x and 2x logical sizes."""
from pixlib import Canvas
from _ship5_aircraft import LAYOUT, core, modules


def assembly(count, frame=0):
    c = Canvas(256, 208)
    for number, sprite in enumerate([core(frame)] + modules(frame)):
        if number > count:
            break
        x, y, w, h = LAYOUT[number]
        c.blit(sprite, 128+x*4-w*2, 104+y*4-h*2)
    return c


def nearest(source, factor):
    w, h = round(source.w*factor), round(source.h*factor)
    out = Canvas(w, h)
    for y in range(h):
        for x in range(w):
            out.set(x, y, source.get(int(x/factor), int(y/factor)))
    return out


def generate():
    sheet = Canvas(1080, 430)
    sheet.rect(0, 0, sheet.w, sheet.h, 'g1')
    for column, count in enumerate((0, 2, 4, 8)):
        c = assembly(count)
        sheet.blit(c, column*270+7, 6)
        sheet.blit(nearest(c, .5), column*270+70, 222)
        sheet.blit(nearest(c, .25), column*270+102, 340)
    sheet.save_png('/tmp/ship5_assemblies.png')
    # Keep a true game-size review alongside a separate enlarged view.
    game = Canvas(256, 112)
    game.rect(0, 0, game.w, game.h, 'g1')
    for column, count in enumerate((0,2,4,8)):
        game.blit(nearest(assembly(count), .25), column*64, 4)
        game.blit(nearest(assembly(count, 1), .25), column*64, 58)
    game.save_png('/tmp/ship5_game_size.png')


if __name__ == '__main__':
    generate()
