#include "ndef.h"
#include <iostream>
#include <stdexcept>
using namespace UFlipper;
void require(bool v,const char *why){if(!v)throw std::runtime_error(why);}
template<class F>void rejects(F f){bool rejected=false;try{f();}catch(const std::runtime_error &){rejected=true;}require(rejected,"Invalid NFC data accepted");}
NfcBytes ok(NfcBytes data={}){data.push_back(0x90);data.push_back(0);return data;}
NfcBytes message(){return {0xd1,1,5,'T',2,'e','n','H','i'};}
struct Card {
 NfcBytes cc{0,15,0x20,0,128,0,255,4,6,0xe1,4,0x7f,0xff,0,0xff};
 NfcBytes ndef;int selected=0;bool app=false;
 explicit Card(NfcBytes data):ndef{uint8_t(data.size()>>8),uint8_t(data.size())}{ndef.insert(ndef.end(),data.begin(),data.end());}
 NfcBytes exchange(const NfcBytes &cmd){
  if(cmd.size()==13){require(cmd==NfcBytes({0,0xa4,4,0,7,0xd2,0x76,0,0,0x85,1,1,0}),"Wrong NDEF application");app=true;return ok();}
  require(app,"No dependent application selection");
  if(cmd.size()==7){selected=cmd[6];require(cmd[5]==0xe1 && (selected==3 || selected==4),"Wrong file selection");return ok();}
  require(cmd.size()==5 && cmd[1]==0xb0 && selected,"Unexpected or state-less APDU");
  const auto &file=selected==3?cc:ndef;const size_t offset=(unsigned(cmd[2])<<8)|cmd[3],size=cmd[4];
  require(offset+size<=file.size(),"Out-of-range file read");return ok(NfcBytes(file.begin()+offset,file.begin()+offset+size));
 }
};
void completeRead(NfcBytes data,unsigned frame){
 Card card(data);Type4NdefReader reader(frame);unsigned count=0;
 while(!reader.complete()){require(++count<4000,"Unbounded NDEF read");auto command=reader.command();require(command.size()<=frame,"APDU exceeds negotiated frame");reader.accept(card.exchange(command));}
 require(reader.message()==data,"NDEF read differs from original");
}
int main(){try{
 auto records=parseNdef(message());require(records.size()==1 && records[0].payload.back()=='i',"Text record not parsed");
 completeRead(message(),240);completeRead({},20);
 NfcBytes large{0xc1,1,0,0,2,0,'x'};large.resize(7+512,'a');completeRead(large,20);
 rejects([]{parseNdef({0x91,1,1,'T',0});});
 rejects([]{parseNdef({0xf1,1,1,'T',0});});
 rejects([]{parseNdef({0xd1,1,100,'T',0});});
 rejects([]{parseNdef({0xd0,1,0,'T'});});
 rejects([]{parseNdef({0xd1,1,0,'T',0});});
 rejects([]{Type4NdefReader reader;reader.accept({0x6a,0x82});});
 rejects([]{Type4NdefReader reader;reader.accept(ok());reader.accept(ok());reader.accept(ok({0xff,0xff}));});
 rejects([]{Card card(message());card.cc[13]=0xff;Type4NdefReader reader;while(!reader.complete())reader.accept(card.exchange(reader.command()));});
 rejects([]{Card card(message());card.cc[2]=0x30;Type4NdefReader reader;while(!reader.complete())reader.accept(card.exchange(reader.command()));});
 rejects([]{Card card(message());Type4NdefReader reader;while(reader.label()!="CHECK NDEF length")reader.accept(card.exchange(reader.command()));reader.accept(ok({0,0}));});
 std::cout<<"NDEF contracts passed: stateful reads, bounded chunks, empty/long messages, malformed metadata/records and changed length\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}}
