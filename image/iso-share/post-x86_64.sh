#!/bin/sh

mkdir -p cdroot/boot

cat >early.cfg <<'EARLYCFG'
search.fs_label "Adelie x86_64" root
set prefix=($root)/boot
EARLYCFG

cat >cdroot/boot/grub.cfg <<'GRUBCFG'
menuentry "Adelie Linux Live (Intel 64-bit)" --class linux --id adelie-live-cd {
        insmod iso9660
        insmod linux
        search --label "Adelie x86_64" --no-floppy --set
        linux ($root)/kernel-x86_64 squashroot=x86_64.squashfs
        initrd ($root)/initrd-x86_64
}

menuentry "Reboot and Try Again" --class reboot --id reboot {
        reboot
}

GRUB_DEFAULT=adelie-live-cd
GRUB_TIMEOUT=10
GRUB_DISTRIBUTOR="Adelie"
GRUBCFG

if ! type grub-mkimage>/dev/null; then
	printf "GRUB image cannot be created.\n"
	exit 1
else
	printf '\033[01;32m * \033[37mInstalling GRUB...\033[00;39m\n'
	grub-mkimage -d target/usr/lib/grub/i386-pc -c early.cfg -v -p boot -o grubcore-stage1.img -O i386-pc biosdisk boot btrfs datetime disk ext2 gfxmenu help iso9660 jfs linux ls luks lvm memdisk nilfs2 normal part_gpt part_msdos png scsi search xfs reboot gfxterm gfxterm_background gfxterm_menu all_video
	cat target/usr/lib/grub/i386-pc/cdboot.img grubcore-stage1.img > cdroot/boot/grubcore.img

	grub-mkimage -d target/usr/lib/grub/x86_64-efi -c early.cfg -p boot -o efi64.exe -O x86_64-efi boot btrfs datetime disk ext2 gfxmenu help iso9660 jfs ls luks lvm memdisk nilfs2 normal part_gpt part_msdos png scsi search xfs linux reboot gfxterm gfxterm_background gfxterm_menu all_video
fi

rm early.cfg

mkdir -p cdroot/System/Library/CoreServices
touch cdroot/System/Library/CoreServices/mach_kernel
cat >cdroot/System/Library/CoreServices/SystemVersion.plist <<PLIST
<plist version="1.0">
<dict>
	<key>ProductBuildVersion</key>
	<string>100</string>
	<key>ProductName</key>
	<string>Adélie Linux</string>
	<key>ProductVersion</key>
	<string>1.0</string>
</dict>
PLIST
#cp disk-label cdroot/System/Library/CoreServices/.disk_label
#echo 'Adélie 64-bit' >cdroot/System/Library/CoreServices/.disk_label.contentDetails
cp efi64.exe cdroot/System/Library/CoreServices/boot.efi

if ! type mkfs.fat>/dev/null || ! type mtools>/dev/null; then
	printf "EFI image cannot be created.\n"
	printf "This CD will boot BIOS systems only.\n"
else
	cat >mtoolsrc <<-MTOOLSRC
	drive A: file="efi64.img"
	MTOOLSRC
	export MTOOLSRC="$PWD/mtoolsrc"
	dd if=/dev/zero of=efi64.img bs=1024 count=1440
	mkfs.fat efi64.img
	mmd A:/EFI
	mmd A:/EFI/BOOT
	mcopy efi64.exe A:/EFI/BOOT/bootx64.efi
	rm efi64.exe mtoolsrc
	mv efi64.img cdroot/efi.img
fi
