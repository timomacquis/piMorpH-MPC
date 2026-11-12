from pathlib import Path
import math
B=Path(__file__).resolve().parents[1]
VERSION='0.1.2'
FOLDER='Timo - VST - πMorpH'
PAGES=['SPACE','MOTION','ENVELOPES','FILTER','LFO','MOD']
WAVES=['SINE','TRIANGLE','RAMP UP','RAMP DOWN','SQUARE','SAMPLE & HOLD','RANDOM SMOOTH','FREE CURVE','RANDOM WALK','RANDOM LOOP','BOUNCE']
DIVISIONS=['1/16','1/8','1/4','1/2','1 BAR','2 BARS','4 BARS']
DEST=['NONE','POSITION X','POSITION Y','PATH RATE','ORBIT RATE','ORBIT WIDTH','ORBIT HEIGHT','PITCH','HP CUTOFF','LP CUTOFF','HP RESONANCE','LP RESONANCE','AMPLITUDE','PAN']
SOURCES=['OFF','LFO 1','LFO 2','AMP ENV','FILTER ENV','VELOCITY','NOTE','MOD WHEEL','PRESSURE']
FIELDS=[]
def add(k,n,lo=0,hi=100,v=0,options=None,unit=''):
 FIELDS.append(dict(key=k,name=n,min=lo,max=hi,default=v,options=options,unit=unit))
add('x','X',0,100,50);add('y','Y',0,100,50)
add('path_mode','PATH MODE',0,3,2,['FIXED','ONE SHOT','LOOP','PING-PONG'])
add('path_rate','PATH RATE',0,6,5,DIVISIONS);add('path_depth','PATH DEPTH',0,100,65)
add('orbit_rate','ORBIT RATE',0,6,6,DIVISIONS);add('orbit_width','ORBIT WIDTH',0,100,6);add('orbit_height','ORBIT HEIGHT',0,100,4);add('orbit_phase','ORBIT PHASE',0,359,0,unit='°');add('path_offset','PATH OFFSET',0,100,0)
for p in ['amp','filter']:
 for suffix,name,lo,hi,v,unit in [('attack','ATTACK',1,10000,35,'ms'),('decay','DECAY',1,10000,2400,'ms'),('sustain','SUSTAIN',0,100,65,'%'),('release','RELEASE',1,10000,500,'ms')]:add(p+'_'+suffix,p.upper()+' '+name,lo,hi,v,unit=unit)
add('hp_cutoff','HP CUTOFF',20,15000,80,unit='Hz');add('hp_res','HP RESONANCE',0,100,10)
add('lp_cutoff','LP CUTOFF',20,15000,6000,unit='Hz');add('lp_res','LP RESONANCE',0,100,15)
add('drive','DRIVE',0,100,0);add('hp_env','HP ENV AMOUNT',-100,100,0);add('lp_env','LP ENV AMOUNT',-100,100,25);add('keytrack','KEYTRACK',0,100,30)
add('filter_model','FILTER CHARACTER',0,2,2,['BYPASS','MS STYLE MK1','MS STYLE MK2'])
add('level','LEVEL',0,100,65);add('pan','PAN',-100,100,0);add('transpose','TRANSPOSE',-24,24,0,unit='st');add('glide','GLIDE',0,1000,0,unit='ms');add('voice_limit','VOICES',1,8,8);add('play_mode','PLAY MODE',0,2,0,['POLY','MONO','LEGATO']);add('bend_range','BEND RANGE',1,24,2,unit='st')
for j in [1,2]:
 p='lfo'+str(j)+'_';add(p+'on','LFO '+str(j)+' ON',0,1,0,['OFF','ON']);add(p+'wave','LFO '+str(j)+' WAVE',0,10,0 if j==1 else 6,WAVES);add(p+'sync','LFO '+str(j)+' CLOCK',0,1,1,['HZ','SYNC']);add(p+'rate','LFO '+str(j)+' RATE',0,1,.5);add(p+'depth','LFO '+str(j)+' DEPTH',0,100,50);add(p+'offset','LFO '+str(j)+' OFFSET',-100,100,0);add(p+'phase','LFO '+str(j)+' PHASE',0,359,0);add(p+'reset','LFO '+str(j)+' RESET',0,1,0,['FREE','NOTE'])
 for k in range(4):add(p+'extra'+str(k),'LFO '+str(j)+' EXTRA '+str(k+1),-100,100,-100 if k==1 else 0)
 for k in range(8):add(p+'c'+str(k),'LFO '+str(j)+' CURVE '+str(k+1),-100,100,round(100*math.sin(k*math.tau/8)))
for i in range(12):
 add('point'+str(i)+'_x','POINT '+str(i+1)+' X',0,100,[12,54,82,27,36,86,61,12,50,50,50,50][i]);add('point'+str(i)+'_y','POINT '+str(i+1)+' Y',0,100,[54,84,36,22,79,58,17,54,50,50,50,50][i])
add('point_count','PATH POINTS',2,12,7);add('point_select','SELECT POINT',0,11,0,[str(i+1) for i in range(12)])
for i in range(8):
 add(f'mod{i}_source',f'SLOT {i+1} SOURCE',0,len(SOURCES)-1,0,SOURCES);add(f'mod{i}_dest',f'SLOT {i+1} DEST',0,len(DEST)-1,0,DEST);add(f'mod{i}_amount',f'SLOT {i+1} AMOUNT',-100,100,0)
add('slot_select','MOD SLOT',0,7,0,[str(i+1) for i in range(8)])
