#!/usr/bin/env python3

import platform, sys, os
import shlex
import shutil
try:
    import lsb_release
    OS_LINUX_HAS_LSB_REL = True
except:
    OS_LINUX_HAS_LSB_REL = False
    pass

PACKAGE_NAME = sys.argv[1]

OS_UNKNOWN = 0
OS_WIN = 1
OS_MAC = 2
OS_LINUX = 3

def win_release():
    uname = platform.uname()
    rel = uname.release
    if not rel: rel = uname.version.split('.', 1)[0]
    return 'Windows-{}'.format(rel)

def mac_release():
    ver = '.'.join(platform.mac_ver()[0].split('.')[:2])
    return 'MacOS-{}'.format(ver)

def linux_os_release():
    items = {}
    with open('/etc/os-release') as distro:
        lexer = shlex.shlex(distro, posix=True)
        lexer.whitespace_split = True
        for line in lexer:
            k, v = line.split('=', 1)
            item[k] = v
    if 'NAME' in items:
        name = items['NAME']
        safe = True
        for ch in ' /\\$':
            if ch in name:
                safe = False
                break
        if safe:
            if 'VERSION_ID' in items:
                return '{NAME}-{VERSION_ID}'.format(*items)
            return name
    return 'Linux'

def linux_release():
    if OS_LINUX_HAS_LSB_REL:
        values = lsb_release.get_distro_information()
        if 'ID' in values and 'RELEASE' in values:
            return '{ID}-{RELEASE}'.format(**values)
    return linux_os_release()


known_oses = {
    'Windows': (OS_WIN, win_release, "zip"),
    'Darwin': (OS_MAC, mac_release, "tar.gz"),
    'Linux': (OS_LINUX, linux_release, "tar.gz")
}

def flatten(system_name):
    os_info = known_oses[system_name]
    return (os_info[0], os_info[1](), os_info[2])

OS_CODE, OS_SUFFIX, PACKAGE_EXT = flatten(platform.system())

with open('build/CMakeCache.txt') as cache:
    for line in cache:
        line = line.split('PROJECT_VERSION', 1)
        if len(line) < 2: continue
        line = line[1]
        if line[0] != ':' and line[0] != '=': continue
        line = line.split('=', 1)
        if len(line) < 2: continue
        PROJECT_VERSION = line[1].strip()

PACKAGE_DEST = '../..'
if OS_CODE == OS_WIN:
    PACKAGE_DEST += '/../artifacts'
PACKAGE_FILENAME = \
    '{PACKAGE_NAME}-{PROJECT_VERSION}-{OS_SUFFIX}.{PACKAGE_EXT}' \
        .format(**globals())

os.chdir('build/out/usr' if OS_CODE == OS_WIN else 'artifacts/usr/local')

# Until I learn how to use conan consitently and could remove the external
PATH_BLACKLIST = [
    'include/date/',
    'lib/cmake/date/',
    'share/cmake/ctre/',
    'include/unicode-db/',
    'include/ctll.hpp',
    'include/ctre.hpp',
    'include/ctll/',
    'include/ctre/',
]

for rm_path in PATH_BLACKLIST:
    try:
        if os.path.isdir(rm_path):
            shutil.rmtree(rm_path)
        else:
            os.remove(rm_path)
    except FileNotFoundError:
        pass

empties = set()
for dirpath, dirnames, filenames in os.walk('.'):
    if len(filenames) == 0 and len(dirnames) == 0:
        empties.add(dirpath)

for dirname in empties: os.removedirs(dirname)

roots = []
for dirpath, dirnames, filenames in os.walk('.'):
    roots = sorted(dirnames)
    break

try:
    os.makedirs(os.path.abspath(PACKAGE_DEST), exist_ok=True)
except:
    pass

print(">>> artifacts/{}".format(PACKAGE_FILENAME))

if PACKAGE_EXT == "zip":
    import zipfile
    archive_mode = 'w'

    class Archive:
        def __init__(self, name, mode):
            self.arch = zipfile.ZipFile(name, mode=mode)
        def append(self, filename):
            if os.path.isdir(filename):
                for dirpath, dirnames, filenames in os.walk(filename):
                    for filename in filenames:
                        self.arch.write(os.path.join(dirpath, filename))
                return
            self.arch.write(filename)
        def close(self): self.arch.close()
elif len(PACKAGE_EXT) > 4 and PACKAGE_EXT[:4] == 'tar.':
    import tarfile
    archive_mode = 'w:'+ PACKAGE_EXT[4:]

    class Archive:
        def __init__(self, name, mode):
            self.arch = tarfile.open(name, mode=mode)
        def append(self, filename): self.arch.add(filename)
        def close(self): self.arch.close()
else:
    raise RuntimeError("Unknown extension: ." + PACKAGE_EXT)

archive = Archive(
    '{PACKAGE_DEST}/{PACKAGE_FILENAME}'.format(**globals()),
    archive_mode)
for root in roots:
    print("    +", root)
    archive.append(root)

archive.close()
