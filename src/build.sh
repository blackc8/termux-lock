#!/bin/sh
pkg install cmake clang
name="termux-lock"
vers="0.1"
appN=$name"_"$vers"_arm"
echo "build $name($vers)"
echo "[+] building binaries..."
cmake .
echo "[+] creating package..."
chmod 0775 -R $appN
cp plogin $appN"/data/data/com.termux/files/usr/bin/plogin"
dpkg-deb -b $appN
mv $appN".deb" ../
echo "[+] build complete."

