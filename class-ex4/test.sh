chr="/home/student/testroot"

mkdir -p $chr
mkdir -p $chr/{bin,lib,lib64}

cp -v /bin/bash $chr/bin
list="$(ldd /bin/bash | egrep -o '/lib.*\.[0-9]')"
for i in $list; do cp -v --parents "$i" "${chr}"; done

cp -v /bin/echo $chr/bin
list="$(ldd /bin/echo | egrep -o '/lib.*\.[0-9]')"
for i in $list; do cp -v --parents "$i" "${chr}"; done

cp -v /bin/ls $chr/bin
list="$(ldd /bin/ls | egrep -o '/lib.*\.[0-9]')"
for i in $list; do cp -v --parents "$i" "${chr}"; done

cd $chr
sudo chroot $chr /bin/bash
