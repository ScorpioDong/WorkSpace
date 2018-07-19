import Speak

def main():
    rate = 22050
    speak = Speak.Speak()
    speak.play_music('mario.data', rate)
