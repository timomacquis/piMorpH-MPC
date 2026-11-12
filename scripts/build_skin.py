"""Original native MPC skin; shared cached artwork, no stock skin redistribution."""
import json,math,hashlib,shutil,copy,os
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont,ImageFilter
from config import B,FIELDS,FOLDER,VERSION,PAGES
from path_art import path_atlas
P=json.loads((B/'params.json').read_text());I={p['key']:i for i,p in enumerate(P)}
assert max(I.values())<4096, 'Force native skin handle limit exceeded'
OUT=B/os.environ.get('PMORPH_SKIN_OUTPUT','build/skin')/FOLDER;A=OUT/'Plugin Skins'
if OUT.exists():shutil.rmtree(OUT)
A.mkdir(parents=True,exist_ok=True);defs=[]
BG='#171c1f';PANEL='#141d24';FIELD='#242c31';INK='#f1f2ed';EDGE='#42515c';AMBER='#c7aa6d'
def font(n):return ImageFont.truetype(str(B/'reference/fonts/TitilliumWeb-Regular.ttf'),n)
def bitmap(name,w,h,caption='',fill=FIELD,ink=INK):
 im=Image.new('RGB',(w,h),BG);d=ImageDraw.Draw(im);d.rounded_rectangle((1,1,w-2,h-2),radius=9,fill=fill,outline=EDGE);d.text((w/2,h/2),caption,font=font(19 if w<140 else 22),fill=ink,anchor='mm');im.save(A/name);return name
def bounds(x,y,w,h,extra=None):return dict(version=2,acceptsHWFocus='Yes',showWhenDataModelInvalid='Hide',whenVisible='Always',boundsType='Absolute',bounds=f'{round(x)} {round(y)} {round(w)} {round(h)}',additionalInvalidatingHandles=extra or [])
def child(kind,data,x,y,w,h):return {'version':2,'componentData':{'version':1,'name':kind,'type':kind,'data':data},'handle remapping':{'version':1,'map':[]},'bounds':bounds(x,y,w,h)}
def gate(key,v,n=2):return f'IndexedEnabling/{v}/{n}/Parameter {I[key]}'
def art(name,x,y,w,h):
 c=child('Image',dict(version=2,imageType='Regular',colour='0',image=name),x,y,w,h);c['bounds']['acceptsHWFocus']='No';return c
def label(x,y,w,h,kind='Value',n=26):
 c=child('Label',dict(version=1,textStyle=dict(version=1,font=dict(version=1,name='Titillium Web',style='Regular',height=int(n)),colour='fff1f2ed',justification='horizontallyCentred verticallyCentred',case='Original'),type=kind,handleName='Data'),x,y,w,h);c['bounds']['acceptsHWFocus']='No';return c
def action(h,event='Mouse Down'):return {'version':2,'onAction':event,'handler':h,'handleName':'Data','additionalData':'','handle remapping':{'version':1,'map':[]}}
def local(name,parts,actions=None,focus_ring=True):
 if actions and focus_ring:
  for part in parts:part['bounds']['acceptsHWFocus']='No'
  w=max(int(c['bounds']['bounds'].split()[0])+int(c['bounds']['bounds'].split()[2]) for c in parts);h=max(int(c['bounds']['bounds'].split()[1])+int(c['bounds']['bounds'].split()[3]) for c in parts)
  fn=f'focus_{w}_{h}.png'
  if not (A/fn).exists():
   im=Image.new('RGBA',(w,h));ImageDraw.Draw(im).rounded_rectangle((2,2,w-3,h-3),radius=8,outline='#ffffff',width=1);im.save(A/fn)
  focus=art(fn,0,0,w,h);focus['bounds']['whenVisible']='WhenFocussed';parts.append(focus)
 clear=dict(version=1,colour='0',image='');defs.append(dict(key=name,value=dict(version=4,actions=actions or [],backgroundData=dict(version=1,focussed=clear,unfocussed=clear),ignoreMousePresses=not bool(actions),disableCoarseDataWheel=False,repeats=1,hideQLinkBounds=True,componentsData=parts)));return name
serial=0
def place(kind,key,x,y,w,h,extra=None):
 c=child(kind,dict(version=1,handleName='Data'),x,y,w,h);c['handle remapping']['map']=[dict(key='Data',value=f'Parameter {I[key]}')];c['bounds']['additionalInvalidatingHandles']=extra or [];return c
LEVEL1_BUTTON_H=48
def toggle(key,title,x,y,w,h,extra=None,level1=True):
 if level1:y+=(h-LEVEL1_BUTTON_H)//2;h=LEVEL1_BUTTON_H
 global serial
 serial+=1;parts=[]
 for state in [0,1]:
  fn=f'button_{title.replace("/","-").replace(" ","_")}_{w}_{h}_{state}.png'
  if not (A/fn).exists():bitmap(fn,w,h,('ON' if state else 'OFF') if key in ['rev_on','del_on','lfo1_on','lfo2_on'] else ('SYNC' if state else 'FREE') if key=='del_sync' else title,'#383122' if state else FIELD,AMBER if state else INK)
  c=art(fn,0,0,w,h);c['bounds']['additionalInvalidatingHandles']=[gate(key,state)];parts.append(c)
 name=local('toggle'+str(serial),parts,[action('Toggle Switch'),action('Toggle Switch','Enter Pressed')]);return place(name,key,x,y,w,h,extra)
def dial(key,title,x,y,w,h,extra=None,slider=False):
 global serial
 serial+=1;fn=f'box_{w}_{h}.png'
 if slider:
  fn=f'mixer_control_{w}_{h}.png';Image.new('RGB',(w,h),PANEL).save(A/fn)
 elif not (A/fn).exists():bitmap(fn,w,h,'',PANEL)
 parts=[art(fn,0,0,w,h)]
 if slider:
  sw=128 if w<200 else w-20;strip='slider.png' if sw==128 else f'slider_{sw}.png'
  if not (A/strip).exists():
   im=Image.new('RGB',(sw,26*128),PANEL);d=ImageDraw.Draw(im)
   for frame in range(128):
    yy=frame*26;xx=5+(sw-10)*frame/127;d.rounded_rectangle((2,yy+10,sw-3,yy+16),radius=3,fill='#55595d');d.line((5,yy+13,xx,yy+13),fill=AMBER,width=5);d.ellipse((xx-6,yy+7,xx+6,yy+19),fill=AMBER)
   im.save(A/strip)
  parts += [child('Knob',dict(version=5,knobType='FilmStrip',filmStrip=strip,numFrames=127,invert=False,dragOrientation='Horizontal',handleName='Data'),10,57,sw,26),label(12,29,w-24,27,n=25)]
 else:parts += [child('Knob',dict(version=5,knobType='FilmStrip',filmStrip='dial.png',numFrames=127,invert=False,dragOrientation='Vertical',handleName='Data'),12,(h-58)//2,58,58),label(86,43,w-94,42,n=30)]
 # Static captions are artwork, avoiding repeated host name queries.
 cap=f'caption_{hashlib.sha256(title.encode()).hexdigest()[:12]}_{w}.png'
 if not (A/cap).exists():
  im=Image.new('RGB',(w-94,25),PANEL);ImageDraw.Draw(im).text(((w-94)/2,12),title,font=font(18),fill=INK,anchor='mm');im.save(A/cap)
 parts.append(art(cap,10 if slider else 86,8,w-16 if slider else w-94,25))
 if slider:
  for c in parts:
   if c['componentData']['type']=='Label':c['componentData']['data']['textStyle']['justification']='horizontallyCentred verticallyCentred'
 return place(local('dial'+str(serial),parts,[action('Q-Link')]),key,x,y,w,h,extra)
# One shared dial and one shared slider strip, used by every page.
for slider in [False,True]:
 w,h=(128,26) if slider else (58,58);im=Image.new('RGB',(w,h*128),PANEL);d=ImageDraw.Draw(im)
 for f in range(128):
  y=f*h
  if slider:
   d.rounded_rectangle((2,y+10,w-3,y+16),radius=3,fill='#55595d');x=5+(w-10)*f/127;d.line((5,y+13,x,y+13),fill=AMBER,width=5);d.ellipse((x-6,y+7,x+6,y+19),fill=AMBER)
  else:
   d.ellipse((4,y+4,54,y+54),outline=INK,width=2);a=math.radians(-135+270*f/127);d.line((29,y+29,29+math.sin(a)*20,y+29-math.cos(a)*20),fill=INK,width=3)
 im.save(A/('slider.png' if slider else 'dial.png'))
Image.new('RGB',(1280,628),BG).save(A/'background.png')
VIOLET='#b195d7';GOLD='#c4ab70';AZURE='#83bed0';BLACK='#05080d'
def title(text,x,y,w=540,n=24):
 fn='title_'+hashlib.sha256((text+str(w)+str(n)).encode()).hexdigest()[:14]+'.png';im=Image.new('RGB',(w,36),BG);ImageDraw.Draw(im).text((0,18),text,font=font(n),fill=INK,anchor='lm');im.save(A/fn);return art(fn,x,y,w,36)
def readout(key,x,y,w,h,n=22):return place(local('read'+str(len(defs)),[label(0,0,w,h,n=n)]),key,x,y,w,h)
MODAL_BUTTON_H=36
modal_manifest=[]
def picker(key,titletext,x,y,w=286,h=48):
 target_h=52 if key in {'play_mode','path_mode','filter_model'} else LEVEL1_BUTTON_H
 y+=(h-target_h)//2;h=target_h
 flag=key+'__open';parts=[]
 for state in [0,1]:
  fn=f'picker_{w}_{h}_{state}.png';bitmap(fn,w,h,'', '#383122' if state else FIELD)
  if state:
   im=Image.open(A/fn);ImageDraw.Draw(im).rounded_rectangle((1,1,w-2,h-2),radius=9,outline=AMBER,width=1);im.save(A/fn)
  c=art(fn,0,0,w,h);c['bounds']['additionalInvalidatingHandles']=[gate(flag,state)];parts.append(c)
 parts.append(readout(key,8,0,w-16,h,22))
 return place(local('picker'+str(len(defs)),parts,[action('Toggle Switch'),action('Toggle Switch','Enter Pressed')]),flag,x,y,w,h)
def option(parts,key,x,y,w,h):parts.append(picker(key,key,x,y,w,h))
def popup(parts,key,anchor):
 flag=key+'__open';cond=[gate(flag,1)];opts=P[I[key]]['options'];captions=[f'{i+1:02d}' for i in range(len(opts))] if key=='point_select' else opts
 cols=min(4,len(opts)) if len(opts)>=10 else 3 if len(opts)>=6 else min(2,len(opts));rows=math.ceil(len(opts)/cols)
 pad=24;gap=12;bh=MODAL_BUTTON_H;bw=max(110,math.ceil(max(font(22).getlength(t) for t in captions))+32)
 panelw=max(420,pad*2+cols*bw+(cols-1)*gap);bw=(panelw-pad*2-(cols-1)*gap)//cols
 gridtop=70;gridh=rows*bh+(rows-1)*gap;footer=gridtop+gridh+22;panelh=footer+bh+pad
 x=(1280-panelw)//2;y=(628-panelh)//2
 # The full-body scrim consumes touches; current values/curves remain visible.
 veil='modal_scrim.png'
 if not (A/veil).exists():Image.new('RGBA',(1280,628),(4,7,13,155)).save(A/veil)
 blocker=local('modal_blocker_'+key,[art(veil,0,0,1280,628)],[action('Toggle Switch')],focus_ring=False)
 c=place(blocker,flag,0,0,1280,628,cond);c['bounds']['acceptsHWFocus']='No';parts.append(c)
 # Raise the actual opener above the veil so its yellow state stays identifiable.
 raised=copy.deepcopy(anchor);raised['bounds']['additionalInvalidatingHandles'].extend(cond);parts.append(raised)
 panel=f'modal_panel_{panelw}_{panelh}.png';bitmap(panel,panelw,panelh,'',PANEL)
 # Blank panel space consumes a tap without reaching a knob behind it.
 surface=local('modal_surface_'+key,[art(panel,0,0,panelw,panelh)],[action('Q-Link')],focus_ring=False)
 parts.append(place(surface,flag,x,y,panelw,panelh,cond))
 parts.append(title(P[I[key]]['name'],x+pad,y+14,panelw-pad*2,22));parts[-1]['bounds']['additionalInvalidatingHandles']=cond
 for op,caption in enumerate(captions):
  tag='compact_op_'+hashlib.sha256((key+str(op)).encode()).hexdigest()[:12]
  for state in [0,1]:bitmap(tag+f'_{state}.png',bw,bh,caption,'#383122' if state else FIELD,AMBER if state else INK)
  native=child('Button',dict(version=2,onImage=tag+'_1.png',offImage=tag+'_0.png',buttonId=op,numButtonsInGroup=len(opts),handleName='Data',gestureBehaviour='Instant'),0,0,bw,bh)
  parts.append(place(local(tag,[native],[action('Q-Link')]),key,x+pad+(op%cols)*(bw+gap),y+gridtop+(op//cols)*(bh+gap),bw,bh,cond))
 parts.append(toggle(flag,'CLOSE',x+(panelw-160)//2,y+footer,160,bh,cond,level1=False))
 modal_manifest.append(dict(key=key,options=len(opts),columns=cols,rows=rows,panel=[x,y,panelw,panelh],button_height=bh,opener=anchor['bounds']['bounds']))
# 256-frame shared glyphs: blank is code 225; live curves refresh only after edits.
for kind,ink in [('wave','#586477'),('path',VIOLET),('segment',VIOLET),('origin',GOLD),('ampgraph',GOLD),('filtergraph',VIOLET),('response',VIOLET),('lfo1graph',GOLD),('lfo2graph',VIOLET)]:
 im=Image.new('RGBA',(8,64*256));d=ImageDraw.Draw(im)
 for frame in range(256):
  code=round(frame*225/255)
  if code==225:continue
  lo,hi=divmod(code,15);y1=frame*64+60-hi*4;y2=frame*64+60-lo*4
  
  if kind=='segment':d.line((0,frame*64+60-lo*4,7,frame*64+60-hi*4),fill=ink,width=1)
  elif kind=='origin':d.ellipse((2,frame*64+58-lo*4,5,frame*64+62-lo*4),fill=ink)
  else:d.rectangle((0,min(y1,y2),7,max(y1,y2)+1),fill=ink)
 im.save(A/(kind+'.png'))
def graph(parts,kind,x,y,w,h,grid=True):
 fn=f'graph_bg_{w}_{h}_{grid}.png'
 if not (A/fn).exists():
  im=Image.new('RGB',(w,h),BLACK);d=ImageDraw.Draw(im);d.rectangle((0,0,w-1,h-1),outline=EDGE)
  if grid:
   for i in range(1,8):d.line((i*w/8,0,i*w/8,h),fill='#1f2633')
   for i in range(1,5):d.line((0,i*h/5,w,i*h/5),fill='#1f2633')
  im.save(A/fn)
 parts.append(art(fn,x,y,w,h))
 for i in range(64):
  c=place(local('graph'+kind+str(i)+str(len(defs)),[child('Knob',dict(version=5,knobType='FilmStrip',filmStrip=kind+'.png',numFrames=255,invert=False,dragOrientation='Vertical',handleName='Data'),0,0,8,h-12)]),f'{kind}{i}',x+8+i*(w-16)/64,y+6,(w-16)/64,h-12)
  c['bounds']['acceptsHWFocus']='No';parts.append(c)
def line_atlas(name,ink):
 if not (A/name).exists():
  atlas=Image.new('RGBA',(6,96*5987));segments=[]
  for a in range(64):
   for b in range(64):segments.append((0,a*95/63,6,b*95/63))
  for mode in ['out','in']:
   for boundary in [0,96]:
    for y in range(64):
     for x in range(7):segments.append((0,y*95/63,x,boundary) if mode=='out' else (x,boundary,6,y*95/63))
  for flip in [False,True]:
   for x0 in range(7):
    for x1 in range(7):segments.append((x0,96 if flip else 0,x1,0 if flip else 96))
  assert len(segments)==5986
  for code,coords in enumerate(segments):
   frame=Image.new('RGBA',(18,288));ImageDraw.Draw(frame).line(tuple(v*3 for v in coords),fill=ink,width=5)
   atlas.paste(frame.resize((6,96),Image.Resampling.LANCZOS),(0,code*96))
  atlas.save(A/name)
 return name
lfo_visuals={}
def precise_lfo(parts,j,x,y):
 fn=line_atlas('lfo_line_'+('gold' if j==1 else 'violet')+'_6x96.png',GOLD if j==1 else VIOLET)
 bg='lfo_precise_bg.png';im=Image.new('RGB',(600,110),'#0b0e18');d=ImageDraw.Draw(im)
 for xx in range(12,589,72):d.line((xx,7,xx,102),fill='#252935')
 d.line((12,55,588,55),fill='#343744');im.save(A/bg);parts.append(art(bg,x,y,600,110))
 if j not in lfo_visuals:
  c=child('Knob',dict(version=5,knobType='FilmStrip',filmStrip=fn,numFrames=5987,invert=False,dragOrientation='Vertical',handleName='Data'),0,0,6,96);c['bounds']['acceptsHWFocus']='No';lfo_visuals[j]=local(f'lfo_visual_{j}',[c])
 for i in range(96):
  c=place(lfo_visuals[j],f'lfo{j}line{i}',x+12+i*6,y+7,6,96);c['bounds']['acceptsHWFocus']='No';parts.append(c)
 # Observation window, not playback state; selectors use the same shape groups as C++.
 for shape in range(11):
  cycles=8 if shape in (5,6,8,9) else 4;caption=str(cycles)+(' CYCLE' if cycles==1 else ' CYCLES');name=f'lfo_window_{cycles}.png'
  if not (A/name).exists():
   image=Image.new('RGB',(140,28),BG);ImageDraw.Draw(image).text((138,14),caption,font=font(18),fill='#aab1bf',anchor='rm');image.save(A/name)
  c=art(name,x+456,y+118,140,28);c['bounds']['additionalInvalidatingHandles']=[gate(f'lfo{j}_wave',shape,11)];parts.append(c)

def precise_filter(parts,x,y):
 # Reuse the exact-size gold/violet LFO atlases; only one extra blue atlas.
 strips=[line_atlas('lfo_line_gold_6x96.png',GOLD),line_atlas('filter_line_petrol_6x96.png','#518397'),line_atlas('lfo_line_violet_6x96.png',VIOLET)]
 name='filter_precise_bg.png';im=Image.new('RGB',(600,340));d=ImageDraw.Draw(im)
 for yy in range(340):
  f=yy/339;d.line((0,yy,599,yy),fill=tuple(round(a+(b-a)*f) for a,b in zip((17,18,31),(10,13,22))))
 for xx in range(12,589,72):d.line((xx,22,xx,309),fill='#272b37')
 for db in [18,0,-24,-48,-60]:
  yy=22+(18-db)/78*287;d.line((12,yy,588,yy),fill='#3b3b49' if db==0 else '#272b37')
 d.text((12,325),'20 Hz',font=font(18),fill='#aab1bf',anchor='lm');d.text((588,325),'20 kHz',font=font(18),fill='#aab1bf',anchor='rm');d.text((300,325),'−60 … +18 dB',font=font(16),fill='#aab1bf',anchor='mm');im.save(A/name);parts.append(art(name,x,y,600,340))
 for curve in range(3):
  for row in range(3):
   for i in range(96):
    n=(curve*3+row)*96+i
    c=place(local('filterline_'+str(n),[child('Knob',dict(version=5,knobType='FilmStrip',filmStrip=strips[curve],numFrames=5987,invert=False,dragOrientation='Vertical',handleName='Data'),0,0,6,96)]),f'filterline{n}',x+12+i*6,y+22+row*96,6,96)
    c['bounds']['acceptsHWFocus']='No';parts.append(c)
 name='filter_legend.png';im=Image.new('RGB',(600,28),BG);d=ImageDraw.Draw(im)
 for xx,text,ink in [(0,'HPF',GOLD),(160,'LPF','#518397'),(320,'HPF → LPF',VIOLET)]:d.line((xx,14,xx+26,14),fill=ink,width=2);d.text((xx+38,14),text,font=font(18),fill=ink,anchor='lm')
 im.save(A/name);parts.append(art(name,x,y+406,600,28))

marker_types={}
def path_key(i):return f'pathseg{i//32}_{i%32}' if i<384 else f'pathtile{i}'
def marker(parts,name,xkey,ykey,x,y,height,ink,ring=False):
 thumb=f'marker_{name}.png'
 if not (A/thumb).exists():
  im=Image.new('RGBA',(12,12));d=ImageDraw.Draw(im);d.ellipse((1,1,10,10),fill=None if ring else ink,outline=ink,width=2);im.save(A/thumb)
 track='marker_track.png'
 if not (A/track).exists():Image.new('RGBA',(588,12)).save(A/track)
 if name not in marker_types:
  c=child('Slider',dict(version=3,sliderType='TrackAndThumb',direction='Horizontal',thumbImage=thumb,thumbTrackVisible=False,revealedImage=track,handleName='Data'),0,0,588,12);c['bounds']['acceptsHWFocus']='No';marker_types[name]=local('marker_'+name,[c])
 rows=65;step=height/(rows-1)
 for row in range(rows):parts.append(place(marker_types[name],xkey,x-6,y+row*step-6,588,12,[gate(ykey,rows-1-row,rows)]))

def precise_space(parts,x,y,motion=False):
 height=256 if motion else 384;tileh=8 if motion else 12;pad=7 if motion else 12;boxh=270 if motion else 410
 bg=f'space_bg_{height}.png';im=Image.new('RGB',(600,boxh));d=ImageDraw.Draw(im)
 for yy in range(boxh):
  f=yy/max(1,boxh-1);d.line((0,yy,599,yy),fill=tuple(round(a+(b-a)*f) for a,b in zip((15,17,28),(8,12,20))))
 for xx in range(0,577,72):d.line((12+xx,pad,12+xx,pad+height-1),fill='#252b38')
 for j in range(7):
  yy=j*height/6;d.line((12,pad+yy,587,pad+yy),fill='#252b38')
 im.save(A/bg);parts.append(art(bg,x,y,600,boxh))
 fine=[]
 if not motion:
  strip=line_atlas('space_wave_6x96.png','#58647788')
  c=child('Knob',dict(version=5,knobType='FilmStrip',filmStrip=strip,numFrames=5987,invert=False,dragOrientation='Vertical',handleName='Data'),0,0,6,96)
  c['bounds']['acceptsHWFocus']='No';wave_type=local('space_wave_tile',[c])
  for i in range(192):
   key=f'wave{i}' if i<64 else f'wavefine{i}';fine.append(place(wave_type,key,x+12+i%96*6,y+12+96+(i//96)*96,6,96))
 strip=f'pathmask_9x{tileh}.png'
 if not (A/strip).exists():path_atlas(A/strip,9,tileh)
 glyph=child('Knob',dict(version=5,knobType='FilmStrip',filmStrip=strip,numFrames=73922,invert=False,dragOrientation='Vertical',handleName='Data'),0,0,9,tileh);glyph['bounds']['acceptsHWFocus']='No';glyph_type=local(f'path_mask_{tileh}',[glyph])
 for row in range(32):
  rowparts=[place(glyph_type,path_key(row*64+col),col*9,0,9,tileh) for col in range(64)]
  c=place(local(f'path_row_{tileh}_{row}',rowparts),f'pathrow{row}',x+12,y+pad+row*tileh,576,tileh,[gate(f'pathrow{row}',1)]);c['bounds']['acceptsHWFocus']='No';fine.append(c)
 parts.append(place(local(f'precise_reveal_{height}',fine),'ui_plot_mode',0,0,1280,628,[gate('ui_plot_mode',0)]))
 # Build-time blur only: no image processing or new notifications during playback.
 blurred=f'space_blurred_{height}.png';blur_image=im.filter(ImageFilter.GaussianBlur(3.2));ImageDraw.Draw(blur_image).text((582,boxh-18),'Processing…',font=font(18),fill='#a4a6b5',anchor='rm');blur_image.save(A/blurred)
 overlay=art(blurred,x,y,600,boxh);overlay['bounds']['additionalInvalidatingHandles']=[gate('ui_plot_mode',1)];parts.append(overlay)
 marker(parts,'origin','x','y',x+12,y+pad,height,GOLD,True)
 marker(parts,'start','path_start_x','path_start_y',x+12,y+pad,height,AZURE)
 if motion:marker(parts,'selected','path_selected_x','path_selected_y',x+12,y+pad,height,'#e4c3ff',True)

def pathgraph(parts,x,y,w,h,background=True):
 if background:
  fn=f'graph_bg_{w}_{h}_True.png'
  if not (A/fn).exists():
   im=Image.new('RGB',(w,h),BLACK);d=ImageDraw.Draw(im);d.rectangle((0,0,w-1,h-1),outline=EDGE)
   for i in range(1,8):d.line((i*w/8,0,i*w/8,h),fill='#1f2633')
   for i in range(1,5):d.line((0,i*h/5,w,i*h/5),fill='#1f2633')
   im.save(A/fn)
  parts.append(art(fn,x,y,w,h))
 for group in range(12):
  for i in range(32):
   c=place(local('pathline'+str(group)+'_'+str(i)+'_'+str(len(defs)),[child('Knob',dict(version=5,knobType='FilmStrip',filmStrip='origin.png' if group==11 else 'segment.png',numFrames=255,invert=False,dragOrientation='Vertical',handleName='Data'),0,0,(w-16)/32,h-12)]),f'pathseg{group}_{i}',x+8+i*(w-16)/32,y+6,(w-16)/32,h-12)
   c['bounds']['acceptsHWFocus']='No';parts.append(c)
def dialgrid(parts,keys):
 for j,key in enumerate(keys):
  x=652+(j%2)*310;y=90+(j//2)*125
  if P[I[key]].get('options'):parts.append(title(P[I[key]]['name'],x,y,294,18));option(parts,key,x,y+40,294,70)
  else:parts.append(dial(key,P[I[key]]['name'],x,y,294,112))
def native_envelope(parts,prefix,x,y,w,h):
 # New original styling; only the public skin component schema is used.
 data=dict(version=1,envelopeType='AHDSR',envelopeStyle='WithoutHeader',envelopeName='',backgroundColour='00000000',scaleColour='3042515c',lineColour='ffc4ab70' if prefix=='amp' else 'ffb195d7',fillColour='0cc4ab70' if prefix=='amp' else '0cb195d7',highlightColour='00000000',lineWidth=2,handleWidth=12,handleHeight=12,handleShape='Circle',handleLineWidth=2,handleLineColour='ffc4ab70' if prefix=='amp' else 'ffb195d7',handleFillColour='ff0e101d',handleTextColour='fff1f2ed',handleLineWidthOnPressed=2,handleLineColourOnPressed='ffffffff',handleFillColourOnPressed='ff242c31',handleTextColourOnPressed='ffffffff',handleArrowLineColourOnPressed='',handleArrowFillColourOnPressed='',delayHandle='Invalid',holdHandle='Invalid',level1Handle='Invalid',attackHandle='Attack',decayHandle='Decay',sustainHandle='Sustain',releaseHandle='Release',attackShapeHandle='Invalid',decay1ShapeHandle='Invalid',decayShapeHandle='Invalid',releaseShapeHandle='Invalid',forceHideShapeHandles=True,forceCurveShape=0.)
 c=child('Envelope',data,x,y,w,h);c['bounds']['acceptsHWFocus']='No';c['bounds']['showWhenDataModelInvalid']='Show'
 c['handle remapping']['map']=[dict(key=k.title(),value=f'Parameter {I[prefix+"_"+k]}') for k in ['attack','decay','sustain','release']]
 parts.append(c)
def graphics_probe(parts,page):
 flag='slot_select__open';closed=[gate(flag,0)];opened=[gate(flag,1)]
 for c in parts[1:]:c['bounds']['additionalInvalidatingHandles'].extend(closed)
 parts.append(toggle(flag,'TEST GRAPHIQUE',652,16,240,36,closed))
 bitmap('native_test_bg.png',1240,600,'', '#0e101d')
 c=art('native_test_bg.png',20,8,1240,600);c['bounds']['additionalInvalidatingHandles']=opened;parts.append(c)
 start=len(parts);parts.append(title('ESSAI DES TRACÉS NATIFS',44,20,1000,22))
 parts.append(title('AMPLITUDE — réglages actuels',44,65,1000,20));native_envelope(parts,'amp',44,104,1150,138)
 parts.append(title('FILTER — réglages actuels',44,258,1000,20));native_envelope(parts,'filter',44,296,1150,138)
 parts.append(title('PATH — tracé fixe de contrôle',44,448,560,19))
 c=child('Path',dict(version=1,handleName='Invalid',path='m 10 90 l 80 20 l 190 76 l 280 10 l 390 92 l 520 30',drawPath=True,colour='ffb195d7',opacity=1.),50,480,540,106);c['bounds']['acceptsHWFocus']='No';c['bounds']['showWhenDataModelInvalid']='Show';parts.append(c)
 parts.append(title('Les enveloppes doivent suivre les potentiomètres.',634,468,560,18));parts.append(title('Le tracé PATH doit former une ligne continue.',634,505,560,18))
 for c in parts[start:]:c['bounds']['additionalInvalidatingHandles'].extend(opened)
 parts.append(toggle(flag,'FERMER',1040,20,190,42,opened))
page_keys=[['x','y','transpose','glide','voice_limit','level','pan','bend_range'],['path_rate','path_depth','orbit_rate','orbit_width','orbit_height','orbit_phase','path_offset','point_count'],[f'{prefix}_{s}' for prefix in ['amp','filter'] for s in ['attack','decay','sustain','release']],['hp_cutoff','hp_res','lp_cutoff','lp_res','drive','hp_env','lp_env','keytrack'],[f'lfo{j}_{k}' for j in [1,2] for k in ['rate','depth','offset','phase']],[f'mod{j}_amount' for j in range(8)]]
tabs=[]
for page,name in enumerate(PAGES):
 parts=[art('background.png',0,0,1280,628)]
 if page==0:
  parts.append(picker('play_mode','PLAY MODE',768,553,326,48));precise_space(parts,20,92)
  parts.append(title('ONE PASS · ORIGIN / START · PATH + ORBIT',30,514,590,18));dialgrid(parts,page_keys[page])
 elif page==1:
  parts.append(picker('path_mode','PATH MODE',768,553,326,48));precise_space(parts,20,92,True)
  parts.append(picker('point_select','POINT',20,384,120,48));parts.extend([dial('point_x','POINT X',152,380,224,110),dial('point_y','POINT Y',388,380,224,110)])
  parts.extend([toggle('add_point','ADD POINT',20,518,284,50),toggle('remove_point','REMOVE POINT',324,518,290,50)]);dialgrid(parts,page_keys[page])
 elif page==2:
  parts.append(title('AMPLITUDE',20,72));native_envelope(parts,'amp',20,114,600,176);parts.append(title('FILTER ENVELOPE',20,318));native_envelope(parts,'filter',20,360,600,176);dialgrid(parts,page_keys[page])
 elif page==3:
  parts.append(picker('filter_model','FILTER CHARACTER',768,553,326,48));parts.append(title('HIGH PASS → LOW PASS',20,74));precise_filter(parts,20,124);parts.append(title('STATIC · SMALL SIGNAL · ENV / MOD OFF',28,488,590,18));dialgrid(parts,page_keys[page])
 elif page==4:
  # Both LFOs share one page, with independent shape and clock selectors.
  for j,y in [(1,72),(2,328)]:
   parts.append(title('LFO '+str(j),20,y,82,22));parts.append(toggle(f'lfo{j}_on','ON',104,y,100,42));parts.append(picker(f'lfo{j}_wave','WAVE',214,y,400,42));precise_lfo(parts,j,20,y+56)
   parts.append(picker(f'lfo{j}_sync','SYNC',20,y+180,166,44));parts.append(picker(f'lfo{j}_reset','RESET',198,y+180,166,44))
  dialgrid(parts,page_keys[page])
  # Deeper waveform controls on native subtabs, all persisted with the preset.
 elif page==5:
  parts.append(title('SOURCE',90,60,260,18));parts.append(title('DESTINATION',412,60,330,18));parts.append(title('AMOUNT',900,60,300,18))
  for j in range(8):
   y=102+j*62;parts.append(title(str(j+1),28,y+4,40,20));parts.append(picker(f'mod{j}_source','SOURCE',80,y,300,52));parts.append(picker(f'mod{j}_dest','DEST',398,y,380,52));
   bitmap('amount_box.png',442,52,'',PANEL)
   if not (A/'amount_track.png').exists():
    im=Image.new('RGBA',(316,28));d=ImageDraw.Draw(im);d.line((5,14,310,14),fill=EDGE,width=3);d.line((158,6,158,22),fill='#85909a',width=1);im.save(A/'amount_track.png')
    im=Image.new('RGBA',(12,26));ImageDraw.Draw(im).rounded_rectangle((1,2,10,23),radius=4,fill=VIOLET,outline=INK,width=1);im.save(A/'amount_thumb.png')
    Image.new('RGBA',(316,28)).save(A/'amount_reveal.png')
   control=child('Slider',dict(version=3,sliderType='TrackAndThumb',direction='Horizontal',thumbImage='amount_thumb.png',thumbTrackVisible=False,revealedImage='amount_reveal.png',handleName='Data'),16,12,316,28)
   visual=[art('amount_box.png',0,0,442,52),art('amount_track.png',16,12,316,28),control,label(342,0,90,52,n=30)]
   parts.append(place(local('amount'+str(j),visual,[action('Q-Link')]),f'mod{j}_amount',802,y,442,52))
  # Short matrix rows use the standard dial with a centred caption/value area.
 # Main content rises into the space freed by the repeated page heading.
 for c in parts[1:]:
  rect=list(map(int,c['bounds']['bounds'].split()));x,y,w,h=rect
  footer=any(m['key']=='Data' and m['value'] in {f'Parameter {I[k+"__open"]}' for k in ['play_mode','path_mode','filter_model']} for m in c['handle remapping']['map'])
  if not footer and ((y>=60 and y<588) or c['componentData']['type'].startswith('precise_reveal_')):
   c['bounds']['bounds']=f'{x} {y-48} {w} {h}'
 # Native compact panels share the same body-wide input barrier.
 anchors={}
 for c in parts:
  for remap in c['handle remapping']['map']:
   idx=int(remap['value'].split()[-1]);key=P[idx]['key']
   if key.endswith('__open'):anchors.setdefault(key[:-6],c)
 for key,anchor in anchors.items():popup(parts,key,anchor)
 # Capacity probe completed; no experimental overlay on this build.
 native=local('page'+str(page),parts);tabs.append(dict(version=3,tabName=name,fnKeyIndex=page,fnKeySubIndex=0,qlinkBoundsData=['0 0 0 0'],componentName=native,initialSize='0 0 1280 628',scale=1.))
# Native LFO subpages expose extras and eight curve anchors without loading more artwork.
for j in [1,2]:
 parts=[art('background.png',0,0,1280,628),title(f'LFO {j} · SHAPE',20,8)]
 for k in range(2):
  # These two controls have real DSP meanings; unused reserved extras stay hidden.
  parts.append(dial(f'lfo{j}_extra{k}', 'WIDTH / STEP / LENGTH' if k==0 else 'EVOLVE / BOUNCE',20+k*620,80,600,112))
 precise_lfo(parts,j,320,218)
 for k in range(8):parts.append(dial(f'lfo{j}_c{k}',f'CURVE {k+1}',20+(k%4)*310,372+(k//4)*118,296,108))
 native=local(f'lfoextra{j}',parts);tabs.append(dict(version=3,tabName=f'LFO {j} SHAPE',fnKeyIndex=4,fnKeySubIndex=j,qlinkBoundsData=['0 0 0 0'],componentName=native,initialSize='0 0 1280 628',scale=1.))
J={'pageData':{'version':1,'info':{'version':1,'type':'CompleteDescription'},'componentDefinitions':{'version':1,'importFiles':['/usr/share/Akai/Content/Synths/Generic/Generic Knob Overlay.json','/usr/share/Akai/Content/Synths/Generic/Generic Menu Overlay.json'],'localComponentDefinitions':defs},'tabs':tabs}}
# Passive single-leaf graph wrappers need no separate native component instance.
# Keep leaf geometry, filmstrip and parameter remapping exactly unchanged.
lookup={d['key']:d['value'] for d in defs};flattened=0
for definition in defs:
 for c in definition['value']['componentsData']:
  model=lookup.get(c['componentData']['type'])
  if not model or model['actions'] or len(model['componentsData'])!=1:continue
  leaf=model['componentsData'][0];rect=list(map(int,leaf['bounds']['bounds'].split()));outer=list(map(int,c['bounds']['bounds'].split()))
  if leaf['componentData']['type'] not in ['Knob','Slider'] or rect!=[0,0,outer[2],outer[3]]:continue
  if leaf['handle remapping']['map'] or leaf['bounds']['additionalInvalidatingHandles']:continue
  c['componentData']=leaf['componentData'].copy();c['bounds']['acceptsHWFocus']='No';flattened+=1
print('Inlined',flattened,'passive graph leaf references; visual assets and geometry unchanged')
(B/'Validation/Native-Graphics/Level1-Standard-Skin/layout.json').write_text(json.dumps(modal_manifest,ensure_ascii=False,indent=2)+'\n')
(A/'TUI.json').write_text(json.dumps(J,ensure_ascii=False,separators=(',',':')))
ql=[]
for page,keys in enumerate(page_keys):
 mapping={f'Q-Link {i}':-1 for i in range(1,17)}
 for physical,key in zip([13,9,5,1,14,10,6,2],keys):mapping[f'Q-Link {physical}']=I[key]
 ql.append({'Tab':page+1,'SubTab':1,'Bank Direction':'Column','Q-Links':mapping})
for j in [1,2]:
 mapping={f'Q-Link {i}':-1 for i in range(1,17)}
 keys=[f'lfo{j}_extra{k}' for k in range(2)]+[f'lfo{j}_c{k}' for k in range(8)]
 for physical,key in zip([13,9,5,1,14,10,6,2,15,11,7,3,16,12,8,4],keys):mapping[f'Q-Link {physical}']=I[key]
 ql.append({'Tab':5,'SubTab':j+1,'Bank Direction':'Column','Q-Links':mapping})
for name in ['Q-Links.json','Q-Links - 8by1.json','Q-Links - 4by4.json']:(A/name).write_text(json.dumps({'version':4,'info':{'version':1,'type':'CompleteDescription'},'Screen Mode Q-Links':{'version':4,'map':ql},'Program Mode Q-Links':ql[0]['Q-Links']},indent=2))
(OUT/'version.xml').write_text('<plugincontent version="1.0"><identifier>Timo.vst.πmorph</identifier><version>0.1.0.0</version></plugincontent>\n')
print('Generated six native pages + two LFO shape subpages;',len(defs),'components;',len(list(A.glob('*.png'))),'cached images')
