#include "protobufplugin.h"
#include "flipperzero/rpc/storagereadoperation.h"
#include <QBuffer>
#include <QCoreApplication>
#include <QDebug>
#include <cmath>
#include <memory>
#include <stdexcept>
void require(bool ok,const char*why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char**argv) {
    QCoreApplication app(argc,argv);
    try {
        ProtobufPlugin encoder;
        for(const bool truncated:{false,true}) {
            QBuffer file;Flipper::Zero::StorageReadOperation read(1,"/ext/fixture",&file);
            read.start();read.encodeRequest(&encoder);
            std::unique_ptr<QObject> stat(encoder.decode(QByteArray::fromHex(truncated?"090801ca01040a021803":"070801ca01020a00")));
            require(bool(stat),"Stat fixture not decoded");read.feedResponse(stat.get());read.encodeRequest(&encoder);
            std::unique_ptr<QObject> chunk(encoder.decode(QByteArray::fromHex("06080152020a00")));
            require(bool(chunk),"Empty chunk fixture not decoded");read.feedResponse(chunk.get());
            require(read.isFinished() && read.isError()==truncated && std::isfinite(read.progress()),"Empty or truncated read handled incorrectly");
            require(truncated || read.progress()==100,"Completed empty read did not reach 100 percent");
        }
        qInfo()<<"PASS: production decoder and RPC read accept empty payload, reject truncated content, keep progress finite";return 0;
    }catch(const std::exception&e){qCritical()<<e.what();return 1;}
}
