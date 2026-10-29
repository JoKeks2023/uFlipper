#include "key_envelope.h"
#include "io_utils.h"
#include <iostream>
#include <fstream>
using namespace UFlipper;
void require(bool ok,const char*why){if(!ok)throw std::runtime_error(why);}
template<class F>void reject(F run){try{run();}catch(const std::exception&){return;}throw std::runtime_error("Invalid envelope accepted");}
int main() {
    auto root=std::filesystem::temp_directory_path()/("uflipper-keys-"+UFlipper::randomId());
    try {
        std::filesystem::create_directory(root);std::filesystem::permissions(root,std::filesystem::perms::owner_all);
        const auto data=FileCrypto::randomKey(),recovery=FileCrypto::randomKey(),wrong=FileCrypto::randomKey();const auto id=KeyEnvelope::randomId();
        auto password=KeyEnvelope::password(data,id,"test password"),key=KeyEnvelope::recovery(data,id,recovery);
        require(KeyEnvelope::inspect(password).memoryBytes==64*1024*1024 && KeyEnvelope::inspect(password).operations==2,"KDF parameters not persisted");
        auto p=KeyEnvelope::unlockPassword(password,id,"test password");auto r=KeyEnvelope::unlockRecovery(key,id,recovery);
        require(p.bytes()==data && r.bytes()==data,"Unlock methods changed the data key");
        auto changed=KeyEnvelope::password(p.bytes(),id,"new test password");
        require(KeyEnvelope::unlockPassword(changed,id,"new test password").bytes()==data,"Password rewrap changed data key");
        reject([&]{KeyEnvelope::unlockPassword(password,id,"wrong");});reject([&]{KeyEnvelope::unlockRecovery(key,id,wrong);});
        reject([&]{KeyEnvelope::unlockPassword(changed,id,"test password");});
        reject([&]{KeyEnvelope::unlockRecovery(key,KeyEnvelope::randomId(),recovery);});
        reject([&]{KeyEnvelope::unlockPassword(key,id,"test password");});
        reject([&]{KeyEnvelope::unlockRecovery(password,id,recovery);});
        auto tampered=key;tampered[61]^=1;reject([&]{KeyEnvelope::unlockRecovery(tampered,id,recovery);});
        tampered=key;tampered.back()^=1;reject([&]{KeyEnvelope::unlockRecovery(tampered,id,recovery);});
        tampered=key;tampered[9]^=1;reject([&]{KeyEnvelope::unlockRecovery(tampered,KeyEnvelope::inspect(tampered).dataKeyId,recovery);});
        tampered=password;tampered[33]=0xff;reject([&]{KeyEnvelope::inspect(tampered);});
        tampered=password;tampered[25]=0xff;reject([&]{KeyEnvelope::inspect(tampered);});
        tampered=password;tampered[44]=0xff;reject([&]{KeyEnvelope::inspect(tampered);});
        tampered=key;tampered[45]=1;reject([&]{KeyEnvelope::inspect(tampered);});
        tampered=key;tampered.push_back(0);reject([&]{KeyEnvelope::inspect(tampered);});
        tampered=key;tampered.pop_back();reject([&]{KeyEnvelope::inspect(tampered);});
        tampered=key;tampered[7]=2;reject([&]{KeyEnvelope::inspect(tampered);});
        reject([&]{KeyEnvelope::inspect({});});reject([&]{KeyEnvelope::password(data,id,"");});
        const auto plain=root/"plain";std::ofstream(plain)<<"host-only encrypted fixture";
        FileCrypto::encrypt(plain,root/"encrypted",data);
        FileCrypto::decrypt(root/"encrypted",root/"from-password",p.bytes());
        FileCrypto::decrypt(root/"encrypted",root/"from-recovery",r.bytes());
        require(hashFile(plain)==hashFile(root/"from-password") && hashFile(plain)==hashFile(root/"from-recovery"),"Independent unlock methods could not decrypt unchanged file");
        auto moved=std::move(p);require(moved.bytes()==data && p.bytes()==SecretKey{},"Moved secret retained old key bytes");
        std::filesystem::remove_all(root);
        std::cout<<"PASS: password/recovery envelopes, unchanged data-key rotation, file decrypt, authenticated metadata, bounded KDF, malformed input, moved secret wipe\n";return 0;
    }catch(const std::exception&e){std::filesystem::remove_all(root);std::cerr<<e.what()<<"\n";return 1;}
}
