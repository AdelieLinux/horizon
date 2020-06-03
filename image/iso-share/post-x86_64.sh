mkdir -p cdroot-x86_64/boot

if ! type grub-mkimage>/dev/null; then
	printf "GRUB image cannot be created.  Using stale copy.\n"
	printf "If you don't have one, this will fail!\n"
else
	printf '\033[01;32m * \033[37mInstalling GRUB...\033[00;39m\n'
	grub-mkimage -c x86/early.cfg64 -v -p boot -o grubcore-stage1.img -O i386-pc biosdisk boot btrfs datetime disk ext2 gfxmenu help iso9660 jfs linux ls luks lvm memdisk nilfs2 normal part_gpt part_msdos png scsi search xfs reboot gfxterm gfxterm_background gfxterm_menu all_video
	cat /usr/lib/grub/i386-pc/cdboot.img grubcore-stage1.img > cdroot-x86_64/boot/grubcore.img

	grub-mkimage -c x86/early.cfg64 -v -p boot -o x86/efi64.exe -O x86_64-efi boot btrfs datetime disk ext2 gfxmenu help iso9660 jfs ls luks lvm memdisk nilfs2 normal part_gpt part_msdos png scsi search xfs linux reboot gfxterm gfxterm_background gfxterm_menu all_video
fi

cp x86/grub.cfg64 cdroot-x86_64/boot/grub.cfg

mkdir -p cdroot-x86_64/System/Library/CoreServices
touch cdroot-x86_64/System/Library/CoreServices/mach_kernel
cat >cdroot-x86_64/System/Library/CoreServices/SystemVersion.plist <<PLIST
<plist version="1.0">
<dict>
	<key>ProductBuildVersion</key>
	<string>1B4</string>
	<key>ProductName</key>
	<string>Adélie Linux</string>
	<key>ProductVersion</key>
	<string>1.0-BETA4</string>
</dict>
PLIST
cp disk-label cdroot-x86_64/System/Library/CoreServices/.disk_label
echo 'Adélie 1.0-BETA4' >cdroot-x86_64/System/Library/CoreServices/.disk_label.contentDetails
cp x86/efi64.exe cdroot-x86_64/System/Library/CoreServices/boot.efi

if ! type mkfs.fat>/dev/null; then
	printf "EFI image cannot be created.\n"
	printf "If one does not already exist, this CD will boot BIOS systems only.\n"
else
	mkdir -p x86/efitemp
	dd if=/dev/zero of=x86/efi64.img bs=1024 count=1440
	mkfs.fat x86/efi64.img
	mount -t vfat -o loop,rw x86/efi64.img x86/efitemp
	mkdir -p x86/efitemp/EFI/BOOT
	mv x86/efi64.exe x86/efitemp/EFI/BOOT/bootx64.efi
	umount x86/efitemp
	rmdir x86/efitemp
fi
cp x86/efi64.img cdroot-x86_64/efi.img
