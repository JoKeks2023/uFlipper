from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import math
OUT=Path(__file__).resolve().parent;S=2
ASSETS=Path(__file__).resolve().parents[4]/'application/assets/fonts'
FONT=str(ASSETS/'ShareTech-Regular.ttf');BOLD=str(ASSETS/'ShareTech-Regular.ttf');MONO=str(ASSETS/'ShareTechMono-Regular.ttf')
C={'bg':'#090400','side':'#000000','white':'#110800','line':'#76380b','text':'#fe8a2c','muted':'#ce5300','accent':'#ff9722','accent_bg':'#3a1f10','green':'#2ed832','selected':'#491d00'}
def font(n=13,bold=False,mono=False):return ImageFont.truetype(MONO if mono else BOLD if bold else FONT,int(n*S))
def new(active,title,subtitle):
 im=Image.new('RGB',(1200*S,760*S),C['bg']);d=ImageDraw.Draw(im)
 def box(bounds,color,r=0,outline=None):
  b=tuple(int(v*S) for v in bounds)
  if r:d.rounded_rectangle(b,radius=r*S,fill=color,outline=outline,width=S)
  else:d.rectangle(b,fill=color,outline=outline,width=S)
 def text(x,y,s,n=13,color=None,bold=False,mono=False):
  ink=color or C['text'];f=font(n,bold,mono)
  if '▾' in s:
   before,after=s.split('▾',1);d.text((x*S,y*S),before+after,font=f,fill=ink)
   at=x*S+d.textlength(before,font=f);cy=(y+n*0.65)*S;d.polygon([(at,cy-2*S),(at+6*S,cy-2*S),(at+3*S,cy+2*S)],fill=ink)
  else:d.text((x*S,y*S),s,font=f,fill=ink)
 def line(x,y,x2,y2,color=None):d.line((x*S,y*S,x2*S,y2*S),fill=color or C['line'],width=S)
 def button(x,y,w,label,primary=False):box((x,y,x+w,y+30),C['accent'] if primary else C['white'],6,C['accent'] if primary else C['line']);text(x+12,y+7,label,12,'#000000' if primary else C['text'])
 box((0,0,1200,42),'#000000');line(0,42,1200,42)
 for x,label in [(1090,'_'),(1125,'[]'),(1165,'X')]:text(x,12,label,15,C['accent'],mono=True)
 text(20,10,'uFlipper',24,bold=True);text(726,15,'ENTWURF · BEISPIELDATEN',11,C['muted'],True)
 
 for xx in range(192,1200,28):line(xx,43,xx,760,'#281306')
 for yy in range(43,760,28):line(192,yy,1200,yy,'#281306')
 box((0,43,192,760),C['side']);line(192,43,192,760)
 text(20,67,'ARBEITSPLATZ',10,C['muted'],True)
 nav=[('Übersicht',99),('Werkzeuge',135),('Bibliothek',171),('Firmware',207),('Backups',243)]
 for label,y in nav:
  if active==label:box((10,y-7,182,y+23),C['selected'],5)
  text(22,y,label,13,bold=active==label)
 text(20,298,'WERKZEUGE',10,C['muted'],True)
 for label,y in [('Infrarot',330),('Sub-GHz',362),('NFC',394),('125-kHz-RFID',426),('iButton / 1-Wire',458),('GPIO und Busse',490),('Apps und Skripte',522)]:
  if active=='Infrarot' and label=='Infrarot':box((10,y-7,182,y+23),C['selected'],5)
  text(22,y,label,12,bold=active==label)
 line(14,661,178,661)
 if active=='Einstellungen':box((10,675,182,707),C['selected'],5)
 text(22,684,'Einstellungen',12,bold=active=='Einstellungen');text(22,728,'Entwicklungsversion',10,C['muted'])
 text(218,64,'Flipper Zero  ▾',12,bold=True);d.ellipse((397*S,68*S,403*S,74*S),fill=C['green']);text(411,64,'USB verbunden',11,C['green']);text(990,64,'1 aktive Aufgabe',11,C['muted']);line(192,95,1200,95)
 text(220,119,title,23,bold=True);text(221,153,subtitle,12,C['muted'])
 return im,d,box,text,line,button

def dashboard():
 im,d,b,t,l,btn=new('Übersicht','Übersicht','Gerät, Versorgung und laufende Arbeit auf einen Blick.')
 b((216,192,932,290),C['white'],7,C['line']);t(234,210,'FLIPPER ZERO',10,C['muted'],True);t(234,234,'Momentum mntm-012',17,bold=True);t(234,264,'Firmware · e1784e74    /    RPC 0.25',11,C['muted'])
 l(660,207,660,274);t(684,210,'AKKU',10,C['muted'],True);t(684,231,'72 %',26,bold=True);t(798,239,'USB · lädt',12,C['green'])
 t(220,319,'Speicher',14,bold=True)
 for y,name,used,total,pct in [(354,'SD-Karte','2,1 GB','14,8 GB',0.14),(410,'Interner Speicher','214 KB','512 KB',0.42)]:
  t(220,y,name,12);t(695,y,used+'  /  '+total,11,C['muted']);b((220,y+25,928,y+29),'#3a1f10',2);b((220,y+25,220+708*pct,y+29),'#fe8a2c',2)
 l(218,465,932,465);t(220,486,'Aktive Sitzung',14,bold=True);b((220,519,930,591),C['white'],5,C['line'])
 t(237,535,'IR-Pulsanalyse',13,bold=True);t(237,560,'Empfang läuft  ·  00:42  ·  automatisch aufgezeichnet',11,C['muted']);btn(830,540,78,'Stoppen')
 t(220,624,'Zuletzt verwendet',14,bold=True)
 for y,name,detail in [(658,'Wohnzimmer.ir','Fernbedienung · 12 Tasten'),(700,'NFC-Aufnahme','Karte · gestern')]:t(220,y,name,12);t(560,y,detail,11,C['muted']);l(220,y+27,930,y+27)
 b((960,96,1200,760),'#100800');l(960,96,960,760);t(980,123,'Versorgung',13,bold=True)
 for y,a,v in [(163,'Ladezustand','Lädt'),(209,'Spannung','4,02 V'),(255,'Strom','+118 mA'),(301,'Temperatur','27 °C'),(347,'Akku-Zustand','98 %')]:t(980,y,a,11,C['muted']);t(980,y+16,v,13);l(979,y+40,1180,y+40)
 t(980,404,'Letzte Sicherung',13,bold=True);t(980,436,'Heute, 10:42',12);t(980,460,'Geprüft · Intern + SD',11,C['green']);btn(980,491,191,'Backup erstellen')
 t(980,565,'Synchronisierung',13,bold=True);t(980,598,'Google Drive',12);t(980,622,'Aktuell',11,C['green']);t(980,683,'Alle Werte sind Beispieldaten.',10,C['muted']);t(980,700,'Keine laufende App-Ansicht.',10,C['muted'])
 im.save(OUT/'01_dashboard.png')

def infrared():
 im,d,b,t,l,btn=new('Infrarot','Infrarot','Empfangen, Pulszeiten untersuchen und Fernbedienungen verwalten.')
 for x,s in [(220,'Empfang'),(319,'Fernbedienung'),(464,'Bibliothek')]:t(x,193,s,12,C['text'] if x==220 else C['muted'],x==220)
 l(218,220,1190,220);b((220,218,287,220),C['accent']);
 b((220,238,932,282),C['accent_bg'],5);t(233,251,'Easy Mode',11,C['accent'],True);t(320,250,'Fernbedienung auf den Flipper richten und eine Taste drücken.',11)
 t(220,307,'Pulsfolge',14,bold=True);t(792,309,'RAW · Mikrosekunden',10,C['muted'])
 b((220,341,932,514),C['white'],5,C['line'])
 for y in [378,420,462]:l(236,y,916,y,'#3a1f10')
 for x in range(250,920,80):l(x,355,x,493,'#3a1f10')
 pts=[];x=240;level=0
 for i in range(42):
  dur=14 if i%5 else 34;y=382 if level else 457;pts.append((x*S,y*S));x+=dur;pts.append((min(x,917)*S,y*S));level=1-level
  if x>=917:break
 d.line(pts,fill=C['accent'],width=2*S)
 t(237,490,'0',9,C['muted']);t(478,490,'10 ms',9,C['muted']);t(730,490,'20 ms',9,C['muted']);t(220,533,'RAW-Frames',14,bold=True)
 for x,s in [(222,'Eingang'),(413,'Pulse'),(558,'Dauer'),(736,'Status')]:t(x,568,s,10,C['muted'],True)
 for y,a,n,duration in [(599,'00:41,812','74','41,2 ms'),(635,'00:40,960','74','41,1 ms'),(671,'00:40,104','74','41,3 ms')]:
  l(220,y-8,930,y-8);t(222,y,a,12,mono=True);t(413,y,n,12,mono=True);t(558,y,duration,12,mono=True);t(736,y,'Empfangen',11,C['green'])
 t(220,729,'Aufzeichnung läuft · Originaldaten werden automatisch gespeichert',11,C['green'])
 b((960,221,1200,760),'#100800');l(960,221,960,760);t(980,242,'Empfang',13,bold=True)
 t(980,278,'Modus',10,C['muted']);b((980,299,1180,332),C['white'],5,C['line']);t(993,309,'RAW-Pulszeiten   ▾',12)
 t(980,354,'Empfänger',10,C['muted']);b((980,375,1180,408),C['white'],5,C['line']);t(993,385,'Flipper intern       ▾',12)
 t(980,439,'Sitzung',13,bold=True);t(980,473,'Dauer',10,C['muted']);t(980,490,'00:42',13,mono=True);t(1090,473,'Lücken',10,C['muted']);t(1090,490,'0',13,mono=True)
 l(980,529,1180,529);t(980,550,'Erweiterte Parameter  +',11,C['muted']);btn(980,603,200,'Empfang stoppen',True);btn(980,648,200,'Aufnahme öffnen');t(980,707,'Pulszeiten, kein optisches',10,C['muted']);t(980,723,'Frequenzspektrum.',10,C['muted'])
 im.save(OUT/'02_infrared.png')

def settings():
 im,d,b,t,l,btn=new('Einstellungen','Einstellungen','Allgemeines, Datenablage und Integrationen.')
 for x,s in [(220,'Allgemein'),(330,'Speicher'),(430,'Cloud'),(515,'Integrationen'),(658,'Über uFlipper')]:t(x,193,s,12,C['text'] if x==220 else C['muted'],x==220)
 l(218,220,1180,220);b((220,218,301,220),C['accent']);t(220,245,'Darstellung und Arbeitsfläche',14,bold=True)
 def row(y,name,value,subtitle=None):
  t(220,y,name,13);b((710,y-7,1074,y+26),C['white'],5,C['line']);t(723,y+2,value+'   ▾',12)
  if subtitle:t(220,y+22,subtitle,11,C['muted'])
  l(220,y+48,1074,y+48)
 row(285,'Design','qFlipper')
 row(348,'Arbeitsfläche','Seitenleiste und Detailbereich','Weitere Modi: Workbench oder Werkzeugfenster.')
 row(416,'Easy Mode','Bei Bedarf im Werkzeug einblenden')
 t(220,497,'Hintergrundbetrieb',14,bold=True);t(220,537,'Bei geschlossenen Fenstern weiterarbeiten',13)
 b((1003,536,1045,560),'#3a1f10',12);d.ellipse((1006*S,539*S,1024*S,557*S),fill='white');t(220,562,'Aktive Geräteaufgaben bleiben über Menüleiste oder Tray sichtbar.',11,C['muted']);l(220,598,1074,598)
 t(220,620,'Lokale Ablage',13,bold=True);t(220,650,'Aufnahmen und Backups im uFlipper-Datenordner',12,C['muted']);btn(873,638,201,'Speicher verwalten')
 l(220,707,1180,707);t(220,726,'uFlipper · Entwicklungsversion',11,C['muted']);t(666,726,'GPLv3    ·    qFlipper / Momentum    ·    Credits',11,C['muted'])
 im.save(OUT/'03_settings.png')
for f in [dashboard,infrared,settings]:f()
print('Created qFlipper-style design proposals. All values are examples.')
