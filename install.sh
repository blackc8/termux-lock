#!/bin/sh
version="0.1"
bd="\033[1m"
dm="\033[2m"
rd="\033[31m"
gr="\033[32m"
yl="\033[33m"
bl="\033[34m"
pl="\033[35m"
wh="\033[37m"
no="\033[0m"

termux_lock_file="termux-lock_"$version"_arm.deb"
termux_lock_deb="https://raw.githubusercontent.com/blackc8/termux-lock/master/"$termux_lock_file

# the banner
printf $bd$wh"########################################\n"
printf "## "$bd$gr"         Termux-lock($no$gr$version$bd$gr)        $no$bd$wh  ##\n"
printf "## "$no$wh"  Lock your termux in a linux way  "$bd$wh"##\n"
printf "##                                    ##\n"
printf "##               GitHub:              ##\n"
printf "## "$bd$yl"github.com/blackc8/termux-lock.git "$bd$wh"##\n"
printf "##          Author: "$bd$bl"@blackc8"$no$bd$wh"          ##\n"
printf $bd$wh"########################################\n"

label(){
  printf $dm$gr"[+] "$no$bd$gr"$1\n"$no
}
msg(){
  printf $dm$bl"[i] "$no$bd$bl"$1\n"$no
}

label "Installing Dependencies"
pkg install -y curl termux-auth

if [ ! -f "$termux_lock_file" ]; then
  label "Fetching termux-lock"
  curl -O $termux_lock_deb
fi

label "Installing termux-lock($version)"
dpkg -i $termux_lock_file

label "Checking password"
if [ ! -f $HOME"/.termux_authinfo" ]
then
  msg "It seems you don't have a password, kindly setup one :)"
  passwd
else
  msg "You already have a password."
  printf $bm$pl"[?]"$bd$pl" do you want to use it(y/n) "
  read -p "" yn
  if [ "$yn" != "y" ] && [ "$yn" != "Y" ]
  then
    msg "Setup a new password"
    passwd
  fi
fi


label "Enabling password login"
if [ ! -d $HOME"/.termux" ]
then
    mkdir ~/.termux
fi
login -e

label "Everything done"
msg "Next time you login a prompt will pop out asking for password"
msg "Have a great day :)"
