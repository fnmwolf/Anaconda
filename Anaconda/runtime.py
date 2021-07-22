if __name__ == '__main__':
    import sys
    import os
    import platform

    frozen = hasattr(sys, 'frozen')

    PROFILE = 0
    DELAYED_LOAD = 0
    LOAD_IMAGES = 1
    CONSOLE_ONLY = 0
    DEBUG = os.getenv('ANACONDA_DEBUG', 0)

    if CONSOLE_ONLY:
        import dummy_pyglet

    if frozen and sys.platform == 'darwin':
        # stupid PyInstaller!
        binary_dir = os.path.dirname(sys.argv[0])
        sys.path.append(binary_dir)
        resources_dir = os.path.join(os.path.split(binary_dir)[0],
            'Resources')
        os.chdir(resources_dir)
        sys.argv = sys.argv[:1] # psn argument?

    import pyglet
    pyglet.options['shadow_window'] = False
    if DEBUG:
        pyglet.options['debug_gl'] = True

    DATA_DIRECTORY = 'data'

    class Logger(object):
        def __init__(self, src, dst):
            self.src = src
            self.dst = dst
            
        def write(self, msg):
            self.src.write(msg)
            self.dst.write(msg)
            self.dst.flush()

    def start_logging():
        try:
            f = open('anaconda.log', 'wb')
        except IOError:
            print '(anaconda.log in use, not logging)'
            return
        stderr = Logger(sys.stderr, f)
        stdout = Logger(sys.stdout, f)
        sys.stderr = stderr
        sys.stdout = stdout
    
    start_logging()

    global old_time
    old_time = None

    try:
        print 'Starting up player...'
        import time

        old_time = time.time()
        def get_dt():
            global old_time
            value = time.time() - old_time
            old_time = time.time()
            return value

        from mmfparser.bytereader import ByteReader
        from mmfparser.data.exe import ExecutableData, findAppendedOffset
        from mmfparser.data.gamedata import GameData
        from mmfparser.player.main import GamePlayer
        from mmfparser.player.common import convert_path
        
        print '(mmfparser took %s)' % get_dt()

        if __name__ == '__main__':
            
            if len(sys.argv) > 1:
                filename = sys.argv[1]
                reader = ByteReader(open(filename, 'rb'))
                
                extension = filename.split('.')[-1]
                
                if extension == 'exe':
                    gameData = ExecutableData(reader, loadImages = LOAD_IMAGES,
                        delayedLoad = DELAYED_LOAD).gameData
                        
                elif extension in ('ccn', 'ccp'):
                    gameData = GameData(reader, loadImages = LOAD_IMAGES,
                        delayedLoad = DELAYED_LOAD)
                        
                elif extension in ('ccj', 'cci'):
                    gameData = GameData(reader, loadImages = LOAD_IMAGES,
                        delayedLoad = DELAYED_LOAD, java = True)
                        
                elif extension == 'ccf':
                    gameData = GameData(reader, loadImages = LOAD_IMAGES,
                        delayedLoad = DELAYED_LOAD, java = True, flash = True)

            elif frozen:
                application_path = 'Application.ccp'
                reader = ByteReader(open(application_path, 'rb'))
                gameData = GameData(reader, loadImages = LOAD_IMAGES,
                    delayedLoad = DELAYED_LOAD)
            else:
                print 'Exiting.'
                raise SystemExit()
    
            if not frozen:
                path = sys.argv[1]
                if extension != 'exe' and os.path.isfile(gameData.editorFilename):
                    path = gameData.editorFilename
                if path is not None:
                    newDirectory = os.path.dirname(path)
                    if newDirectory:
                        sys.path.append(newDirectory)
                        try:
                            os.chdir(newDirectory)
                        except OSError:
                            pass

            newPlayer = GamePlayer(gameData)
            
            if PROFILE:
                import cProfile
                cProfile.run('newPlayer.start()', 'mmfplayer.prof')
                import pstats
                p = pstats.Stats('mmfplayer.prof')
                p.sort_stats('time')
                p.print_stats(40)
                import code
                code.interact(local = locals())
            else:
                if False:
                    import sys
                    f_trace = open('ftrace', 'wb')
                    def trace(frame, event, arg):
                        f_trace.write(
                            "%s, %s:%d" % (event, frame.f_code.co_filename, 
                                frame.f_lineno))
                        f_trace.flush()
                        return trace
                    sys.settrace(trace)

                newPlayer.start()

    except:
        import traceback
        traceback.print_exc()