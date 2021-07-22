import sys
sys.path.append('../../')

from mmfparser.data.mfa import (MFA, Backdrop, FrameItem, ChunkList,
                                ItemFolder, FrameInstance)
from mmfparser.bytereader import ByteReader
from mmfparser.data.chunkloaders.imagebank import ImageItem
from mmfparser.data.chunkloaders.objectinfo import BACKDROP
from mmfparser.data.chunkloaders.frame import NONE_PARENT
from mmfparser.player.dialog import open_file_selector
from PIL import Image

import sys
import os

BIN_DIR = os.path.dirname(os.path.realpath(sys.argv[0]))

if hasattr(sys, 'frozen'):
    DATA_DIR = os.path.realpath(os.path.join(BIN_DIR, '..'))
else:
    DATA_DIR = BIN_DIR

SETS_PATH = os.path.join(DATA_DIR, u'sets')

class Tiler(object):
    def __init__(self, filename):
        self.filename = filename

        self.tiles = os.stat(filename).st_size != 0

        name = os.path.basename(filename).encode('utf-8')
        if self.tiles:
            self.name, size = name.split('_')[:2]
            size = size.split(',')
            self.x_size = int(size[0])
            self.y_size = int(size[1])
        else:
            self.name = name.split('_')[0]

        self.outdir = os.path.splitext(filename)[0]
        try:
            os.makedirs(self.outdir)
        except OSError:
            pass

        self.objects = {}

        template_path = os.path.join(DATA_DIR, 'template.mfa')
        self.mfa = MFA(ByteReader(open(template_path, 'rb')))

        self.object_id = 32
        self.image_id = 0
        self.icon_id = max(self.mfa.icons.itemDict) + 1

    def run(self):
        image_path = os.path.join(SETS_PATH, '%s.png' % self.name)
        image = Image.open(image_path).convert('RGBA')

        col = image.getpixel((0, 0))
        if col[3] == 255:
            print 'Filtering image with transparent color', col
            buf = image.load()
            for y in xrange(image.size[1]):
                for x in xrange(image.size[0]):
                    test_col = buf[(x, y)]
                    if test_col != col:
                        continue
                    buf[(x, y)] = (0, 0, 0, 0)

        frame = self.mfa.frames[0]

        if self.tiles:
            self.run_tiles(image)
        else:
            self.run_special(image)

        frame.folders = []
        for item in frame.items:
            folder = frame.new(ItemFolder)
            folder.items = [item.handle]
            frame.folders.append(folder)

        basename = os.path.basename(self.filename)
        out_name = os.path.splitext(basename)[0] + '.mfa'
        out_path = os.path.join(self.outdir, out_name)

        self.mfa.write(ByteReader(open(out_path, 'wb')))
        print 'Created %s' % out_name

    def run_special(self, image):
        mask_path = os.path.join(SETS_PATH, '%s_mask.png' % self.name)
        mask = Image.open(mask_path).convert('RGBA')

        mask = mask.convert('P')
        colors = mask.getcolors()
        transparent = mask.getpixel((0, 0))

        buf = image.load()

        for (_, color) in colors:
            if color == transparent:
                continue
            color_image = mask.point(lambda x: 1 if x == color else 0)
            box = color_image.getbbox()
            height = box[3] - box[1]
            width = box[2] - box[0]
            new_image = Image.new('RGBA', (width, height), (255, 255, 255, 0))

            buf2 = color_image.load()
            buf3 = new_image.load()

            for y in xrange(height):
                for x in xrange(width):
                    xx = x + box[0]
                    yy = y + box[1]
                    color = buf2[(xx, yy)]
                    if color == 0:
                        continue
                    buf3[(x, y)] = buf[(xx, yy)]

            name = '%s_%s' % (self.name, self.image_id + 1)
            obj = self.create_object(name, None, new_image)
            self.create_instance(obj, box[0], box[1])

    def run_tiles(self, image):
        index_path = os.path.join(SETS_PATH, 'index.txt')
        charmap = open(index_path, 'rb').read()

        index = 0
        for y in xrange(0, image.size[1], self.y_size):
            for x in xrange(0, image.size[0], self.x_size):
                part = image.crop((x, y, x + self.x_size, y + self.y_size))
                char = charmap[index]
                name = '%s_%s_%s' % (self.name,
                                     x / self.x_size,
                                     y / self.y_size)
                self.create_object(name, char, part)
                index += 1

        data = open(self.filename, 'rb').read()

        x = 0
        y = 0
        for line in data.splitlines():
            for c in line:
                if c == ' ':
                    x += self.x_size
                    continue
                frameitem = self.objects[c]
                self.create_instance(frameitem, x, y)
                x += self.x_size

            x = 0
            y += self.y_size

    def create_instance(self, frameitem, x, y):
        frame = self.mfa.frames[0]
        instance = frame.new(FrameInstance)
        instance.x = x
        instance.y = y
        instance.handle = len(frame.instances) + 1
        instance.flags = 0
        instance.parentType = NONE_PARENT
        instance.itemHandle = frameitem.handle
        instance.parentHandle = 0
        instance.layer = 0
        frame.instances.append(instance)

    def create_image(self, image, icon):
        if icon:
            bank = self.mfa.icons
            handle = self.icon_id
            self.icon_id += 1
            image = image.resize((32, 32), Image.LANCZOS)
        else:
            bank = self.mfa.images
            handle = self.image_id
            self.image_id += 1

        item = bank.new(ImageItem, debug=True)
        item.handle = handle

        item.checksum = 123
        item.references = 0
        item.width = image.size[0]
        item.height = image.size[1]
        item.xHotspot = item.yHotspot = item.actionX = item.actionY = 0
        item.flags['Alpha'] = True
        item.transparent = (0, 0, 0)
        item.graphicMode = 4

        item.image = image.tobytes('raw', 'RGBA')
        item.alpha = image.tobytes('raw', 'A')

        bank.items.append(item)
        bank.itemDict[item.handle] = item
        return item

    def create_object(self, name, key, image):
        item = self.create_image(image, False)
        icon = self.create_image(image, True)

        frame = self.mfa.frames[0]
        frameitem = frame.new(FrameItem)
        frame.items.append(frameitem)

        frameitem.name = name
        frameitem.objectType = BACKDROP
        frameitem.handle = self.object_id
        self.object_id += 1
        frameitem.transparent = True
        frameitem.inkEffect = frameitem.inkEffectParameter = 0
        frameitem.antiAliasing = False
        frameitem.iconHandle = icon.handle
        frameitem.chunks = frameitem.new(ChunkList)

        obj = frameitem.new(Backdrop)
        obj.obstacleType = 0
        obj.collisionType = 0
        obj.handle = item.handle

        frameitem.loader = obj

        if key is not None:
            self.objects[key] = frameitem

        return frameitem

def main():
    filenames = open_file_selector('Select .txt file', multi=True,
                                   filters=[('Text files', ('*.txt',)),
                                            ('All Files', ('*.*',)),])
    if not filenames:
        return
    for filename in filenames:
        tiler = Tiler(filename)
        tiler.run()

if __name__ == '__main__':
    main()
