import argparse, struct
from pathlib import Path
from cryptography.hazmat.primitives import serialization,hashes
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.exceptions import InvalidSignature
p=argparse.ArgumentParser()
p.add_argument('--key',required=True);p.add_argument('--input',required=True);p.add_argument('--output',required=True)
a=p.parse_args()
key=serialization.load_pem_private_key(Path(a.key).read_bytes(),password=None)
data=Path(a.input).read_bytes()
assert data and data[0]==0xE9,'Expected an ESP8266 firmware image'
signature=key.sign(data,padding.PKCS1v15(),hashes.SHA256())
key.public_key().verify(signature,data,padding.PKCS1v15(),hashes.SHA256())
try:
 key.public_key().verify(signature,data+b'X',padding.PKCS1v15(),hashes.SHA256())
except InvalidSignature: pass
else: raise RuntimeError('Tampering test failed')
out=Path(a.output);out.parent.mkdir(parents=True,exist_ok=True)
out.write_bytes(data+signature+struct.pack('<I',len(signature)))
print('Signed image created; signature and tampering checks passed.')
