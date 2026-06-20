Used ffmpeg to convert from 8k to 16k sample rate
example: ffmpeg.exe -i not-connected-8k-16bit.wav -ar 16000 not-connected-16k-16bit.wav

Used a ratio of 20.  No idea if that is a good choice or not.
example: wav2sbcArray.exe not-connected-16k-16bit.wav 20