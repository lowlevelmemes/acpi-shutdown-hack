KERNEL_HDD = disk.hdd

.PHONY: clean all run test-kernel

all: $(KERNEL_HDD)

run: $(KERNEL_HDD)
	qemu-system-x86_64 -m 2G -hda $(KERNEL_HDD)

test-kernel:
	$(MAKE) -C test-kernel

$(KERNEL_HDD): test-kernel
	rm -f $(KERNEL_HDD)
	dd if=/dev/zero bs=1M count=0 seek=64 of=$(KERNEL_HDD)
	parted -s $(KERNEL_HDD) mklabel gpt
	parted -s $(KERNEL_HDD) mkpart primary 2048s 100%
	echfs-utils -g -p0 $(KERNEL_HDD) quick-format 512
	echfs-utils -g -p0 $(KERNEL_HDD) import test-kernel/kernel.elf kernel.elf
	echfs-utils -g -p0 $(KERNEL_HDD) import limine.cfg limine.cfg
	limine-install $(KERNEL_HDD)

clean:
	rm -f $(KERNEL_HDD)
	$(MAKE) -C test-kernel clean
