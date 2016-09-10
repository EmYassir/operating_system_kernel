.PHONY: clean all cksum initcksum deploy

all:
	$(MAKE) -C kernel

clean:
	$(MAKE) clean -C kernel
	$(MAKE) clean -C user

# Deploiement du noyau dans la machine virtuelle
# sous MacOS, changer la destination en ~/Library/VirtualBox/TFTP/
deploy: all
	/bin/cp -f kernel/kernel.bin ~/.VirtualBox/TFTP
