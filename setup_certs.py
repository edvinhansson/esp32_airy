import os
import shutil
import subprocess
import sys

def create_ca(output_dir: str) -> int:
    return subprocess.check_call([
        'openssl', 'req',
        '-x509',
        '-newkey', 'rsa:4096',
        '-keyout', os.path.join(output_dir, 'rootca.key'),
        '-out', os.path.join(output_dir, 'rootca.crt'),
        '-sha256',
        '-days', str(3650),
        '-nodes',
        '-subj', '/C=US/ST=Oregon/L=Portland/O=AIRY/OU=Org/CN=www.example.com',
    ], cwd=os.path.abspath('.'), stdout=subprocess.DEVNULL)

def create_and_sign_csr(output_dir: str, name: str):
    subprocess.check_call([
        'openssl', 'req',
        '-new',
        '-nodes',
        '-out', os.path.join(output_dir, f'{name}.csr'),
        '-newkey', 'rsa:4096',
        '-keyout', os.path.join(output_dir, f'{name}.key'),
        '-subj', f'/CN={name}/C=US/ST=Oregon/L=Portland/O=AIRY/OU=Org/CN=www.example.com',
    ], cwd=os.path.abspath('.'), stdout=subprocess.DEVNULL)

    subprocess.check_call([
        'openssl', 'x509',
        '-req',
        '-in', os.path.join(output_dir, f'{name}.csr'),
        '-CA', os.path.join(output_dir, 'rootca.crt'),
        '-CAkey', os.path.join(output_dir, 'rootca.key'),
        '-CAserialout',
        '-out', os.path.join(output_dir, f'{name}.crt'),
        '-days', str(3650),
        '-sha256',
    ], cwd=os.path.abspath('.'), stdout=subprocess.DEVNULL)

    return 0

def main() -> int:
    openssl_dir = 'openssl'
    if not os.path.isdir(openssl_dir):
        os.mkdir(openssl_dir)

    if not os.path.isfile(os.path.join(openssl_dir, 'rootca.key')):
        create_ca(openssl_dir)
    
    if not os.path.isfile(os.path.join(openssl_dir, 'nodered.key')):
        create_and_sign_csr(openssl_dir, 'nodered')

    if not os.path.isfile(os.path.join(openssl_dir, 'grafana.key')):
        create_and_sign_csr(openssl_dir, 'grafana')

    certs_dir = os.path.join('main', 'certs')
    if not os.path.isdir(certs_dir):
        os.mkdir(certs_dir)

    shutil.copyfile(os.path.join(openssl_dir, 'rootca.crt'), os.path.join(certs_dir, 'rootca.crt'))

    return 0

if __name__ == '__main__':
    sys.exit(main())