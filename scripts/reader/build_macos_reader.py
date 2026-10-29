#!/usr/bin/env python3
# Build a reviewable test bundle only. Never install/register a system driver.
import hashlib,json,plistlib,shutil,subprocess,sys,urllib.request
from pathlib import Path
if sys.platform!='darwin':raise SystemExit('This experimental reader target requires macOS')
if len(sys.argv)!=2:raise SystemExit('Usage: build_macos_reader.py NEW_OUTPUT_DIRECTORY')
out=Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=False)
lock=json.loads(Path(__file__).with_name('source-lock.json').read_text())
source=out/'source';source.mkdir()
for name,entry in lock['files'].items():
    payload=urllib.request.urlopen(entry['url'],timeout=45).read()
    if hashlib.sha256(payload).hexdigest()!=entry['sha256']:raise SystemExit('Source hash mismatch: '+name)
    file=source/name;file.parent.mkdir(parents=True,exist_ok=True);file.write_bytes(payload)
def patch(name,old,new):
    file=source/name;text=file.read_text()
    if old not in text:raise SystemExit('Pinned patch no longer applies: '+name)
    file.write_text(text.replace(old,new))
# Never listen on a network interface, including the legacy entry point.
patch('src/ifd-vpcd/ifd-vpcd.c','''    ctx[slot] = vicc_init(hostname, Channel+slot);''','''    ctx[slot] = vicc_init("127.0.0.1", 35963);''')
patch('src/ifd-vpcd/ifd-vpcd.c','unsigned int MaxAPDUDataSize = 0x10000;','unsigned int MaxAPDUDataSize = 0; /* short APDUs only */')
patch('src/ifd-vpcd/ifd-vpcd.c','''    unsigned char *rapdu = NULL;''','''    if (TxLength < 4 || TxLength > 240) return IFD_NOT_SUPPORTED;
    unsigned char *rapdu = NULL;''')
# Fix partial/closed socket handling and bound blocking I/O to five seconds.
patch('src/vpcd/vpcd.c','if (r < 0)\n            return r;','if (r <= 0)\n            return -1;')
start=(source/'src/vpcd/vpcd.c').read_text();a=start.index('    return recv(sock, buffer,',start.index('ssize_t recvall('));b=start.index('\n}',a)
old=start[a:b]
patch('src/vpcd/vpcd.c',old,'''    size_t received = 0;
    while (received < size) {
        ssize_t n = recv(sock, (char *)buffer + received, size - received, 0);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        received += (size_t)n;
    }
    return (ssize_t)received;''')
patch('src/vpcd/vpcd.c','struct addrinfo hints, *res = NULL, *cur;','struct addrinfo hints, *res = NULL, *cur;')
patch('src/vpcd/vpcd.c','''		close(sock);
	}

err:
	freeaddrinfo(res);''','''        close(sock); sock = INVALID_SOCKET;
    }

err:
    if (res) freeaddrinfo(res);
    if (sock != INVALID_SOCKET) {
        int yes = 1;
        struct timeval timeout = {5, 0};
        setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof yes);
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);
    }''')
patch('src/vpcd/vpcd.c','''    size = ntohs(size);

    if (0 != size)''','''    size = ntohs(size);
    if (size > 256) { errno = EMSGSIZE; return -1; }

    if (0 != size)''')
patch('src/vpcd/lock.c','    r = 0;','    int r = 0;')
patch('src/vpcd/lock.c','''        *io_lock = PTHREAD_MUTEX_INITIALIZER;''','''        if (pthread_mutex_init(io_lock, NULL) != 0) { free(io_lock); return NULL; }''')
bundle=out/'uFlipperReader.bundle';contents=bundle/'Contents';(contents/'MacOS').mkdir(parents=True);(contents/'Resources').mkdir()
info={'CFBundleIdentifier':'com.jorisconrad.flipperzeroutil.reader','CFBundleName':'uFlipper Reader (experimental)',
 'CFBundleExecutable':'libuflipperreader.dylib','CFBundlePackageType':'BNDL','CFBundleVersion':'0.1.0','CFBundleShortVersionString':'0.1.0',
 'ifdManufacturerString':'uFlipper / virtualsmartcard','ifdProductString':'uFlipper NFC Reader',
 'ifdCapabilities':'0x00000000','ifdProtocolSupport':'0x00000002','ifdVersionNumber':'0x00000001',
 'ifdVendorID':['0x0483'],'ifdProductID':['0x5740'],'ifdFriendlyName':['uFlipper NFC Reader'],
 'Copyright':'GPLv3-or-later; derived from virtualsmartcard by Frank Morgner. See Resources.'}
(contents/'Info.plist').write_bytes(plistlib.dumps(info))
shutil.copy2(source/'COPYING',contents/'Resources/COPYING.virtualsmartcard')
for name in ('ifdhandler.h','reader.h'):shutil.copy2(source/'headers'/name,contents/'Resources'/name)
sdk=Path(subprocess.check_output(['xcrun','--sdk','macosx','--show-sdk-path'],text=True).strip())
binary=contents/'MacOS/libuflipperreader.dylib'
command=['xcrun','clang','-dynamiclib','-arch','arm64','-arch','x86_64','-mmacosx-version-min=13.0',
 '-DRESPONSECODE_DEFINED_IN_WINTYPES_H','-DHAVE_PTHREAD=1','-DHAVE_ARPA_INET_H=1','-DHAVE_DECL_SO_NOSIGPIPE=1','-DVPCDSLOTS=1',
 '-I'+str(source/'headers'),'-I'+str(sdk/'System/Library/Frameworks/PCSC.framework/Headers'),'-I'+str(source/'src/vpcd'),
 str(source/'src/ifd-vpcd/ifd-vpcd.c'),str(source/'src/vpcd/vpcd.c'),str(source/'src/vpcd/lock.c'),'-framework','PCSC','-o',str(binary)]
subprocess.run(command,check=True)
manifest={'version':1,'experimental':True,'installed':False,'source_lock':lock,
 'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),
 'architectures':subprocess.check_output(['lipo','-archs',str(binary)],text=True).strip().split(),
 'bundle_identifier':info['CFBundleIdentifier'],'minimum_macos':'13.0','endpoint':'127.0.0.1:35963',
 'usb_trigger':{'vendor':'0483','product':'5740'},'maximum_command_bytes':240,'maximum_response_bytes':256,
 'patches':['fixed reverse loopback endpoint and one slot','short APDUs only','bounded socket I/O and response size','partial/closed sockets fail','POSIX mutex initialization'],
 'acceptance':'Not an OS slot or independent-client proof; physical card/driver integration still required.'}
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
(contents/'Resources/source-lock.json').write_text(json.dumps(lock,indent=2)+'\n')
# Keep the final binary digest outside signed resources to avoid a circular hash.
subprocess.run(['codesign','--force','--sign','-',str(bundle)],check=True)
subprocess.run(['codesign','--verify','--strict',str(bundle)],check=True)
manifest['binary_sha256']=hashlib.sha256(binary.read_bytes()).hexdigest()
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
subprocess.run(['ditto','-c','-k','--sequesterRsrc','--keepParent',str(bundle),str(out/'uFlipperReader-test.zip')],check=True)
print(json.dumps({'bundle':str(bundle),'binary_sha256':manifest['binary_sha256'],'architectures':manifest['architectures'],'installed':False}))
