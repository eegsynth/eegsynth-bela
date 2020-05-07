# EEGsynth for Bela

[Bela](https://bela.io) is an embedded computing platform for creating amazing interactive applications. 

This [EEGsynth](https://github.com/eegsynth/eegsynth) repository contains C/C++ and Pure data projects for the Bela, combined with the corresponding EEGsynth patches. 

## How to connect the Bela to the internet

To use git and GitHub in combination with your Bela you have to get it connected to the internet. Connecting it to yout local computer as 192.168.6.2 is not sufficient. If you have a wired ethernet cabl;e, you can just plug it in and reboot with 

    ssh root@bela
    reboot now

Other possibilities are to use a [wifi USB dongle](https://github.com/BelaPlatform/Bela/wiki/Connecting-Bela-to-wifi) or to set up [internet sharing](https://forum.bela.io/d/33-internet-sharing-on-osx-ethernet/10) on your computer such that trafic on the local network between your computer and the Bela gets routed to the internet.

## How to install this on the Bela

If you want to contribute to this repository, we recommend you make a fork to your own GitHub account and follow these instructions.

    ssh root@bela.local
    ssh-keygen
    cat .ssh/id_rsa.pub 

Copy the SSH key over to GitHub (your profile, settings, SSH and GPG keys). 

    git clone git@github.com:username/eegsynth-bela.git

You can also skip authentication on GitHub and use http to make a clone of the eegsynth version.

    ssh root@bela.local
    git clone https://github.com/eegsynth/eegsynth-bela.git

Now that you have a copy of the git repository on your bela, you can use symbolic links to access all or some of the projects here.

    cd ~/Bela
    rm -rf projects  # WARNING, only do this after backing up your projects
    ln -s ~/eegsynth-bella/projects .

Or if you want to mix some eegsynth-bela projects with your own you can do

    cd ~/Bela/project
    ln -s ~/eegsynth-bella/projects/someproject .
    ln -s ~/eegsynth-bella/projects/anotherproject .


