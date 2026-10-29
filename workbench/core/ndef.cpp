#include "ndef.h"
#include <algorithm>
#include <stdexcept>
namespace UFlipper {
namespace {
unsigned be16(const NfcBytes &b, size_t offset) {
    if (offset + 2 > b.size()) throw std::runtime_error("Truncated NDEF metadata");
    return (unsigned(b[offset]) << 8) | b[offset+1];
}
NfcBytes readBinary(unsigned offset, unsigned count) {
    if (offset > 0x7fff || !count || count > 255) throw std::runtime_error("Unsupported READ BINARY range");
    return {0,0xb0,uint8_t(offset>>8),uint8_t(offset),uint8_t(count)};
}
NfcBytes slice(const NfcBytes &b, size_t &offset, size_t length) {
    if (length > b.size()-offset) throw std::runtime_error("Truncated NDEF record");
    NfcBytes result(b.begin()+offset,b.begin()+offset+length);offset+=length;return result;
}
}
std::vector<NdefRecord> parseNdef(const NfcBytes &b) {
    if (b.size()>32765) throw std::runtime_error("NDEF message exceeds supported limit");
    std::vector<NdefRecord> records;
    size_t p=0;bool ended=false;
    while(p<b.size()) {
        if(records.size()>=1024 || b.size()-p<3) throw std::runtime_error("Invalid NDEF record header");
        const auto flags=b[p++];const auto tnf=uint8_t(flags&7);
        const bool mb=flags&0x80, me=flags&0x40, cf=flags&0x20, sr=flags&0x10, il=flags&8;
        if(ended || mb!=records.empty() || cf || tnf==6 || tnf==7)
            throw std::runtime_error("Invalid or unsupported chunked NDEF message");
        const unsigned typeLength=b[p++];
        uint32_t payloadLength=0;
        const auto bytes=sr?1U:4U;
        if(b.size()-p<bytes) throw std::runtime_error("Truncated NDEF payload length");
        for(unsigned i=0;i<bytes;++i) payloadLength=(payloadLength<<8)|b[p++];
        unsigned idLength=0;
        if(il) {if(p==b.size()) throw std::runtime_error("Truncated NDEF identifier length");idLength=b[p++];}
        if((tnf==0 && (typeLength || payloadLength || idLength || il)) || (tnf>=1 && tnf<=4 && !typeLength) || (tnf==5 && typeLength))
            throw std::runtime_error("Invalid NDEF record type");
        NdefRecord record{tnf,slice(b,p,typeLength),slice(b,p,idLength),slice(b,p,payloadLength)};
        records.push_back(std::move(record));ended=me;
        if(me && p!=b.size()) throw std::runtime_error("Trailing data after NDEF message end");
    }
    if(!records.empty() && !ended) throw std::runtime_error("Missing NDEF message end");
    return records;
}
Type4NdefReader::Type4NdefReader(unsigned limit) {
    if(limit<13 || limit>240) throw std::runtime_error("Unsupported NFC frame limit");
    chunk_=std::min(128U,limit-2);
}
unsigned Type4NdefReader::readSize() const {
    if(stage_==ReadCC) return std::min({chunk_,15U,ccLength_-unsigned(cc_.size())});
    if(stage_==ReadMessage) return std::min({chunk_,mle_,length_-unsigned(message_.size())});
    return 2;
}
NfcBytes Type4NdefReader::command() const {
    switch(stage_) {
    case SelectApplication:return {0,0xa4,4,0,7,0xd2,0x76,0,0,0x85,1,1,0};
    case SelectCC:return {0,0xa4,0,0x0c,2,0xe1,3};
    case ReadCCLength:return readBinary(0,2);
    case ReadCC:return readBinary(unsigned(cc_.size()),readSize());
    case SelectNdef:return {0,0xa4,0,0x0c,2,uint8_t(file_>>8),uint8_t(file_)};
    case ReadLength:case CheckLength:return readBinary(0,2);
    case ReadMessage:return readBinary(2+unsigned(message_.size()),readSize());
    case Done:throw std::runtime_error("NDEF read is already complete");
    }
    throw std::runtime_error("Invalid NDEF reader stage");
}
std::string Type4NdefReader::label() const {
    switch(stage_) {
    case SelectApplication:return "SELECT NDEF application";
    case SelectCC:return "SELECT capability container";
    case ReadCCLength:return "READ capability length";
    case ReadCC:return "READ capability container";
    case SelectNdef:return "SELECT NDEF file";
    case ReadLength:return "READ NDEF length";
    case ReadMessage:return "READ NDEF message";
    case CheckLength:return "CHECK NDEF length";
    case Done:return "NDEF complete";
    }
    return {};
}
void Type4NdefReader::parseCC() {
    if(cc_.size()<15 || be16(cc_,0)!=ccLength_ || (cc_[2]>>4)!=2)
        throw std::runtime_error("Only NDEF Type 4 mapping version 2 is supported");
    mle_=be16(cc_,3);
    if(mle_<15 || be16(cc_,5)<1) throw std::runtime_error("Invalid NDEF capability limits");
    size_t p=7;
    while(p<cc_.size()) {
        const auto tag=cc_[p++];
        if(tag==0)continue;
        if(tag==0xfe)break;
        if(p==cc_.size())throw std::runtime_error("Truncated NDEF capability TLV");
        const auto count=cc_[p++];
        if(count>cc_.size()-p)throw std::runtime_error("Truncated NDEF capability TLV value");
        if(tag==4 && count==6 && cc_[p+4]==0 && !file_) {
            file_=be16(cc_,p);fileSize_=be16(cc_,p+2);
            if(file_==0 || file_==0xffff || file_==0xe103 || fileSize_<2)
                throw std::runtime_error("Invalid NDEF file capability");
        }
        p+=count;
    }
    if(!file_)throw std::runtime_error("No freely readable Type 4 NDEF file");
}
void Type4NdefReader::accept(const NfcBytes &response) {
    if(complete())throw std::runtime_error("Unexpected response after NDEF completion");
    if(response.size()<2 || response[response.size()-2]!=0x90 || response.back()!=0)
        throw std::runtime_error("NDEF card command failed or application is unavailable");
    const NfcBytes body(response.begin(),response.end()-2);
    const bool reading=stage_==ReadCCLength || stage_==ReadCC || stage_==ReadLength || stage_==ReadMessage || stage_==CheckLength;
    if(reading && body.size()!=readSize())throw std::runtime_error("Short or oversized NDEF read response");
    switch(stage_) {
    case SelectApplication:stage_=SelectCC;break;
    case SelectCC:stage_=ReadCCLength;break;
    case ReadCCLength:
        ccLength_=be16(body,0);
        if(ccLength_<15 || ccLength_>4096)throw std::runtime_error("NDEF capability container exceeds supported limit");
        stage_=ReadCC;break;
    case ReadCC:
        cc_.insert(cc_.end(),body.begin(),body.end());
        if(cc_.size()==ccLength_) {parseCC();stage_=SelectNdef;}
        break;
    case SelectNdef:stage_=ReadLength;break;
    case ReadLength:
        length_=be16(body,0);
        if(length_>fileSize_-2 || length_>32765)throw std::runtime_error("NDEF length exceeds file or supported limit");
        stage_=length_?ReadMessage:CheckLength;break;
    case ReadMessage:
        message_.insert(message_.end(),body.begin(),body.end());
        if(message_.size()==length_)stage_=CheckLength;
        break;
    case CheckLength:
        if(be16(body,0)!=length_)throw std::runtime_error("NDEF file changed during read");
        parseNdef(message_);stage_=Done;break;
    case Done:break;
    }
}
}
