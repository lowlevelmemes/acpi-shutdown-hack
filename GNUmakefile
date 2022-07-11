.PHONY: all
all: disk.hdd

.PHONY: run-hdd
run: disk.hdd
	qemu-system-x86_64 -M q35 -m 2G -hda disk.hdd

.PHONY: run-hdd-uefi
run-uefi: ovmf-x64 disk.hdd
	qemu-system-x86_64 -M q35 -m 2G -bios ovmf-x64/OVMF.fd -hda disk.hdd

ovmf-x64:
	mkdir -p ovmf-x64
	cd ovmf-x64 && curl -o OVMF-X64.zip https://efi.akeo.ie/OVMF/OVMF-X64.zip && 7z x OVMF-X64.zip

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v3.0-branch-binary --depth=1
	make -C limine

.PHONY: test-kernel
test-kernel:
	$(MAKE) -C test-kernel

disk.hdd: limine test-kernel
	rm -f disk.hdd
	dd if=/dev/zero bs=1M count=0 seek=64 of=disk.hdd
	parted -s disk.hdd mklabel gpt
	parted -s disk.hdd mkpart ESP fat32 2048s 100%
	parted -s disk.hdd set 1 esp on
	limine/limine-deploy disk.hdd
	sudo losetup -Pf --show disk.hdd >loopback_dev
	sudo mkfs.fat -F 32 `cat loopback_dev`p1
	mkdir -p img_mount
	sudo mount `cat loopback_dev`p1 img_mount
	sudo mkdir -p img_mount/EFI/BOOT
	sudo cp -v test-kernel/kernel.elf limine.cfg limine/limine.sys img_mount/
	sudo cp -v limine/BOOTX64.EFI img_mount/EFI/BOOT/
	sync
	sudo umount img_mount
	sudo losetup -d `cat loopback_dev`
	rm -rf loopback_dev img_mount

.PHONY: clean
clean:
	rm -rf iso_root disk.hdd
	$(MAKE) -C test-kernel clean

.PHONY: distclean
distclean: clean
	rm -rf limine ovmf-x64
	$(MAKE) -C test-kernel distclean
