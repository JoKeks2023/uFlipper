#!/usr/bin/env python3
# Direct, unprivileged IFD contract. This is NOT an OS slot/OpenSC acceptance test.
import ctypes as C,hashlib,json,socket,threading,sys
from pathlib import Path
root=Path(sys.argv[1]);binary=root/'uFlipperReader.bundle/Contents/MacOS/libuflipperreader.dylib'
manifest=json.loads((root/'manifest.json').read_text())
assert hashlib.sha256(binary.read_bytes()).hexdigest()==manifest['binary_sha256']
server=socket.socket();server.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1);server.bind(('127.0.0.1',35963));server.listen(1);server.settimeout(10)
requests=[];errors=[];stopping=threading.Event();present=True
def receive_exact(conn,n):
    result=b''
    while len(result)<n:
        data=conn.recv(n-len(result))
        if not data:raise EOFError()
        result+=data
    return result
def peer():
    try:
        while not stopping.is_set():
            conn,_=server.accept();conn.settimeout(10)
            with conn:
                while not stopping.is_set():
                    try:
                        n=int.from_bytes(receive_exact(conn,2),'big');data=receive_exact(conn,n);requests.append(data.hex())
                        if data==b'\x04':response=bytes.fromhex('3b800181') if present else b''
                        elif len(data)==1:continue
                        else:response=bytes.fromhex('6a82')
                        frame=len(response).to_bytes(2,'big')+response
                        # Split the wire frame to exercise exact-length receiving.
                        conn.sendall(frame[:1]);conn.sendall(frame[1:])
                    except EOFError:break
    except Exception as e:
        if not stopping.is_set():errors.append(str(e))
thread=threading.Thread(target=peer,daemon=True);thread.start()
lib=C.CDLL(str(binary));D=C.c_uint32;B=C.c_ubyte
class Pci(C.Structure):_fields_=[('Protocol',D),('Length',D)]
lib.IFDHCreateChannelByName.argtypes=[D,C.c_char_p];lib.IFDHCreateChannelByName.restype=C.c_int32
lib.IFDHICCPresence.argtypes=[D];lib.IFDHICCPresence.restype=C.c_int32
lib.IFDHPowerICC.argtypes=[D,D,C.POINTER(B),C.POINTER(D)];lib.IFDHPowerICC.restype=C.c_int32
lib.IFDHTransmitToICC.argtypes=[D,Pci,C.POINTER(B),D,C.POINTER(B),C.POINTER(D),C.POINTER(Pci)];lib.IFDHTransmitToICC.restype=C.c_int32
lib.IFDHCloseChannel.argtypes=[D];lib.IFDHCloseChannel.restype=C.c_int32
try:
    assert lib.IFDHCreateChannelByName(0,b'uFlipper NFC Reader')==0
    assert lib.IFDHICCPresence(0)==615
    atr=(B*36)();length=D(36)
    assert lib.IFDHPowerICC(0,500,atr,C.byref(length))==0 and bytes(atr[:length.value])==bytes.fromhex('3b800181')
    tx=(B*5).from_buffer_copy(bytes.fromhex('00a4040000'));rx=(B*256)();n=D(256);pci=Pci()
    assert lib.IFDHTransmitToICC(0,Pci(1,8),tx,5,rx,C.byref(n),C.byref(pci))==0 and bytes(rx[:n.value])==bytes.fromhex('6a82')
    oversized=(B*241)();n=D(256)
    assert lib.IFDHTransmitToICC(0,Pci(1,8),oversized,241,rx,C.byref(n),C.byref(pci))==614
    length=D(36);assert lib.IFDHPowerICC(0,502,atr,C.byref(length))==0
    length=D(36);assert lib.IFDHPowerICC(0,501,atr,C.byref(length))==0
    present=False;assert lib.IFDHICCPresence(0)==616
    assert not errors,errors
    result={'success':True,'binary_sha256':manifest['binary_sha256'],'wire_requests':requests,'synthetic_peer':True,'os_registration':False,'independent_pcsc_client':False}
    (root/'contract-result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
finally:
    lib.IFDHCloseChannel(0);stopping.set();server.close()
