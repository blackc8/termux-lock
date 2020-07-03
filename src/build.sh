#!/bin/sh
pkg install clang termux-elf-cleaner
name="termux-lock"
vers="0.1"
appN=$name"_"$vers"_arm"
echo "build $name($vers)"
echo "[+] building binaries..."
gcc plogin.c -l termux-auth -o plogin
echo "[+] Cleaning..."
termux-elf-cleaner plogin
echo "[+] creating package..."
chmod 0775 -R $appN
cp plogin $appN"/data/data/com.termux/files/usr/bin/plogin"
dpkg-deb -b $appN
mv $appN".deb" ../
echo "[+] build complete."

