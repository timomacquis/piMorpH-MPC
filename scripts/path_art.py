"""Original cached occupancy tiles. Build-time rasterization only."""
from PIL import Image,ImageDraw
LUT=[max(0,min(255,round((v-35)*255/220))) for v in range(256)]
def path_atlas(path,width,height):
 atlas=Image.new('RGBA',(width,height*73922))
 points=[(x,y) for x in range(10) for y in range(13)];code=1
 for a in range(130):
  for b in range(a+1,130):
   x,y=points[a];xx,yy=points[b];tile=Image.new('RGBA',(width*3,height*3))
   ImageDraw.Draw(tile).line((x*3,y*height/12*3,xx*3,yy*height/12*3),fill='#b195d7',width=5)
   atlas.paste(tile.resize((width,height),Image.Resampling.LANCZOS),(0,height*code));code+=1
 assert code==8386
 for mask in range(65536):
  small=Image.new('L',(4,4));small.putdata([255 if mask&(1<<bit) else 0 for bit in range(16)])
  alpha=small.resize((width,height),Image.Resampling.LANCZOS).point(LUT)
  tile=Image.new('RGBA',(width,height),'#b195d7');tile.putalpha(alpha);atlas.paste(tile,(0,(8386+mask)*height))
 atlas.save(path)


def draft_atlas(path,width,height):
 atlas=Image.new('RGBA',(width,height*4096))
 for mask in range(4096):
  small=Image.new('L',(3,4));small.putdata([255 if mask&(1<<bit) else 0 for bit in range(12)])
  alpha=small.resize((width,height),Image.Resampling.LANCZOS).point(LUT)
  tile=Image.new('RGBA',(width,height),'#b195d7');tile.putalpha(alpha);atlas.paste(tile,(0,mask*height))
 atlas.save(path)
