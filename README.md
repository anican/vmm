# VMM
Minimal Type-2 hypervisor built on top of KVM.

## Installation
```
ls -l /dev/kvm

sudo apt update
sudo apt install -y build-essential git gdb

# add yourself to the kvm group
sudo usermod -aG kvm $USER
```
Check KVM group permissions. Try logging out/in if it doesn't work right away.
```
getent group kvm
```
