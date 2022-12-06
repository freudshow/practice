import numpy as np
import scipy.signal as scisig
import PIL.ImageFont as pilfont
import PIL.Image as pilimage
import PIL.ImageDraw as pildraw

fnt=pilfont.truetype('Consolas',size=16)
im=pilimage.new('L',(16,16))
draw=pildraw.Draw(im)
draw.text((0,0),chr(0x66f9),font=fnt,fill=255)
x=np.array(im)
print(x)
im.save('cao.png')
ans=np.zeros(32,dtype=np.uint8)
for i in range(16):
    for j in range(2):
        for k in range(8):
            if(x[j*8+k,i]):
                ans[j*16+i]|=(1<<k)

print(list(ans))
