import snakesound
snakesound.open()

print 'opened'

def wee(sound):
    print 'DONE'

# sound = snakesound.Sound(open('imteapot.midi', 'rb').read())
sound = snakesound.Sound(filename = 'hey.ogg')
print 'Duration:', sound.duration
sound.set_callback(wee, sound)
sound.play(0)

import code
code.interact(local = locals())