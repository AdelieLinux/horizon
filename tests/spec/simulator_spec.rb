require 'spec_helper'

def run_simulate(extra = '')
    run_command 'hscript-simulate ' + IFILE_PATH + extra
end

RSpec.describe 'HorizonScript Simulator', :type => :aruba do
    context "argument passing" do
        it "requires an installfile to be specified" do
            run_command 'hscript-simulate'
            expect(last_command_started).to have_output(/usage/)
        end
        it "supports Strict Mode" do
            run_command 'hscript-simulate foo -s'
            expect(last_command_started).to_not have_output(/usage/)
        end
        it "doesn't output ANSI colours when instructed not to" do
            run_command 'hscript-simulate foo -n'
            expect(last_command_started).to_not have_output(/\033/)
        end
        it "doesn't output ANSI colours when redirected" do
            run_command 'hscript-simulate foo 2>/dev/null'
            expect(last_command_started).to_not have_output(/\033/)
        end
        it "outputs the hashbang when redirected" do
            run_command 'hscript-simulate foo'
            expect(last_command_started.stdout).to start_with("#!/bin/sh")
        end
    end
    it "handles validation failures correctly" do
        use_fixture '0024-numeric-hostname.installfile'
        run_simulate
        expect(last_command_started.stderr).to include("Script failed.  Stop.")
    end
    context "simulating 'disklabel' execution" do
        it "creates Apple Partition Maps correctly" do
            use_fixture '0122-disklabel-apm.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("parted -ms /dev/sda mklabel apm")
        end
        it "creates GUID Partition Tables correctly" do
            use_fixture '0124-disklabel-gpt.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("parted -ms /dev/sda mklabel gpt")
        end
        it "creates MBR tables correctly" do
            use_fixture '0123-disklabel-mbr.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("parted -ms /dev/sda mklabel mbr")
        end
    end
    context "simulating 'lvm_pv' execution" do
        it "creates a physical volume" do
            use_fixture '0163-lvmpv-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("pvcreate --force /dev/sdb2")
        end
    end
    context "simulating 'lvm_vg' execution" do
        it "creates a volume group" do
            use_fixture '0166-lvmvg-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("vgcreate MyVolGroup /dev/sdb2")
        end
    end
    context "simulating 'lvm_lv' execution" do
        it "creates a logical volume" do
            use_fixture '0171-lvmlv-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("lvcreate -l 100%FREE -n root MyVolGroup")
        end
        it "supports percentage sizing" do
            use_fixture '0212-lvmlv-size-percent.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("lvcreate -l 50%VG -n root MyVolGroup")
        end
        it "supports byte sizing" do
            use_fixture '0213-lvmlv-size-bytes.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("lvcreate -L 104857600B -n root MyVolGroup")
        end
    end
    context "simulating 'fs' execution" do
        it "creates ext2 filesystems correctly" do
            use_fixture '0193-fs-ext2.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("mkfs.ext2 -q -z /tmp/undo-sdb1 /dev/sdb1")
        end
        it "creates ext3 filesystems correctly" do
            use_fixture '0194-fs-ext3.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("mkfs.ext3 -q -z /tmp/undo-sdb1 /dev/sdb1")
        end
        it "creates ext4 filesystems correctly" do
            use_fixture '0179-fs-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("mkfs.ext4 -q -z /tmp/undo-sdb1 /dev/sdb1")
        end
        it "creates JFS filesystems correctly" do
            use_fixture '0195-fs-jfs.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("mkfs.jfs -q /dev/sdb1")
        end
        it "creates VFAT filesystems correctly" do
            use_fixture '0196-fs-vfat.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("mkfs.vfat -F32 /dev/sdb1")
        end
        it "creates XFS filesystems correctly" do
            use_fixture '0197-fs-xfs.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("mkfs.xfs -f /dev/sdb1")
        end
    end
    context "simulating 'mount' execution" do
        it "mounts directories in tree order" do
            use_fixture '0057-many-mounts.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("mount /dev/sda1 /target/
mkdir -p /target/etc
printf '%s\\t%s\\t%s\\t%s\\t0\\t1\\n' /dev/sda1 / auto defaults >> /target/etc/fstab
mount /dev/gwyn/home /target/home
printf '%s\\t%s\\t%s\\t%s\\t0\\t0\\n' /dev/gwyn/home /home auto defaults >> /target/etc/fstab
mount /dev/sda2 /target/usr
printf '%s\\t%s\\t%s\\t%s\\t0\\t0\\n' /dev/sda2 /usr auto defaults >> /target/etc/fstab
mount /dev/gwyn/source /target/usr/src
printf '%s\\t%s\\t%s\\t%s\\t0\\t0\\n' /dev/gwyn/source /usr/src auto defaults >> /target/etc/fstab")
        end
        it "handles options correctly" do
            use_fixture '0075-mount-options.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("
mount /dev/sda1 /target/
mkdir -p /target/etc
printf '%s\\t%s\\t%s\\t%s\\t0\\t1\\n' /dev/sda1 / auto defaults >> /target/etc/fstab
mount -o relatime /dev/gwyn/home /target/home
printf '%s\\t%s\\t%s\\t%s\\t0\\t0\\n' /dev/gwyn/home /home auto relatime >> /target/etc/fstab
mount /dev/sda2 /target/usr
printf '%s\\t%s\\t%s\\t%s\\t0\\t0\\n' /dev/sda2 /usr auto defaults >> /target/etc/fstab
mount -o noatime /dev/gwyn/source /target/usr/src
printf '%s\\t%s\\t%s\\t%s\\t0\\t0\\n' /dev/gwyn/source /usr/src auto noatime >> /target/etc/fstab")
        end
    end
    context "simulating 'hostname' execution" do
        it "sets the hostname properly" do
            use_fixture '0074-hostname-large.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("hostname heartbreak-is-the-national-anthem")
            use_fixture '0001-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("hostname test.machine")
        end
        it "sets the domain name properly" do
            use_fixture '0074-hostname-large.installfile'
            run_simulate
            expect(last_command_started.stderr).to include("set domain name 'we-sing-it-proudly.new-romantics.club'")
        end
    end
    context "simulating 'arch' execution" do
        it "sets the architecture properly" do
            use_fixture '0223-arch-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("printf 'ppc64\\n' > /target/etc/apk/arch")
        end
    end
    context "simulating 'repository' execution" do
        it "outputs default repositories when none are specified" do
            use_fixture '0001-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("echo 'https://distfiles.adelielinux.org/adelie/stable/system' >> /target/etc/apk/repositories")
        end
        it "outputs requested repositories when specified" do
            use_fixture '0055-repository-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("echo 'https://distfiles.adelielinux.org/adelie/current/system' >> /target/etc/apk/repositories")
            expect(last_command_started.stdout).to_not include("echo 'https://distfiles.adelielinux.org/adelie/stable/system' >> /target/etc/apk/repositories")
        end
    end
    context "simulating 'netssid' execution" do
        it "outputs the network block correctly" do
            use_fixture '0067-netssid-spaces-wpa.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("network={\n\tssid=\"The New Fox 5G\"\n\tpsk=\"shh, sekrit!\"\n\tpriority=5\n}")
        end
    end
    context "simulating 'netaddress' execution" do
        it "configures addressing correctly" do
            use_fixture '0042-netaddress-valid-static6.installfile'
            run_simulate
            # The end quote is missing deliberately.
            expect(last_command_started.stdout).to include('config_eth0="2600:1702:2a80:1b9f:5bbc:af4c:5dd1:a183/64')
        end
        it "configures routing correctly" do
            use_fixture '0048-netaddress-gateway4.installfile'
            run_simulate
            # The end quote is missing deliberately.
            expect(last_command_started.stdout).to include('routes_eth0="default via 172.16.1.1')
        end
        it "doesn't perform OpenRC configuration when netifrc isn't installed" do
            use_fixture '0042-netaddress-valid-static6.installfile'
            run_simulate
            expect(last_command_started.stdout).to_not include ("/target/etc/init.d/net")
        end
        # Runner.Execute.netaddress.OpenRC
        it "performs OpenRC configuration when netifrc is installed" do
            use_fixture '0200-netaddress-openrc.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("ln -s /etc/init.d/net.lo /target/etc/init.d/net.eth0")
        end
    end
    context "simulating 'nameserver' execution" do
        it "configures nameservers correctly" do
            use_fixture '0183-nameserver-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("printf 'nameserver %s\\n' 172.16.1.1 >>/target/etc/resolv.conf")
        end
        it "uses resolv.conf when no interfaces use DHCP" do
            use_fixture '0183-nameserver-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to_not include("mv /target/etc/resolv.conf /target/etc/resolv.conf.head")
        end
        it "uses resolv.conf.head when interfaces use DHCP" do
            use_fixture '0221-nameserver-dhcp.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("mv /target/etc/resolv.conf /target/etc/resolv.conf.head")
        end
    end
    context "simulating 'network' execution" do
        it "copies SSID information" do
            use_fixture '0222-complete.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("cp /target/etc/wpa_supplicant/wpa_supplicant.conf /etc/wpa_supplicant/wpa_supplicant.conf")
        end
        it "copies nameserver information" do
            use_fixture '0222-complete.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("cp /target/etc/resolv.conf* /etc/")
        end
    end
    context "simulating 'signingkey' execution" do
        it "downloads remote keys to target" do
            use_fixture '0186-signingkey-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("curl -L -o /target/etc/apk/keys/packages@adelielinux.org.pub https://distfiles.adelielinux.org/adelie/packages@adelielinux.org.pub")
        end
        it "copies local keys to target" do
            use_fixture '0187-signingkey-local.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("cp /etc/apk/keys/packages@adelielinux.org.pub /target/etc/apk/keys/packages@adelielinux.org.pub")
        end
        it "uses default keys when none are specified" do
            use_fixture '0001-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("cp /etc/apk/keys/packages@adelielinux.org.pub /target/etc/apk/keys/packages@adelielinux.org.pub")
        end
    end
    context "simulating 'pkginstall' execution" do
        # Runner.Execute.pkginstall.APKDB
        it "initialises the APK database" do
            use_fixture '0001-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("apk --root /target --initdb add")
        end
        # Runner.Execute.pkginstall
        it "updates the local repository cache" do
            use_fixture '0001-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("apk --root /target update")
        end
        # Runner.Execute.pkginstall
        it "installs the requested packages" do
            use_fixture '0001-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("apk --root /target add adelie-base")
        end
    end
    context "simulating 'language' execution" do
        # Runner.Execute.language
        it "doesn't configure language by default" do
            use_fixture '0001-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to_not include("/target/etc/profile.d/language.sh")
        end
        it "configures language correctly" do
            use_fixture '0116-language-country.installfile'
            run_simulate
            # leading space also serves as regression test for 1d89a45
            expect(last_command_started.stdout).to include(" ie_IE > /target/etc/profile.d/language.sh")
        end
        it "ensures profile script is executable" do
            use_fixture '0116-language-country.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("chmod a+x /target/etc/profile.d/language.sh")
        end
    end
    context "simulating 'keymap' execution" do
        # Runner.Execute.keymap
        it "configures the system keymap correctly" do
            use_fixture '0178-keymap-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("XKBLAYOUT=us")
        end
    end
    context "simulating 'timezone' execution" do
        # Runner.Execute.timezone.Missing
        it "configures UTC by default" do
            use_fixture '0001-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("ln -s /usr/share/zoneinfo/UTC /target/etc/localtime")
        end
        # Runner.Execute.timezone
        it "configures the system timezone correctly" do
            use_fixture '0134-timezone-subtz.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("ln -s /usr/share/zoneinfo/Pacific/Galapagos /target/etc/localtime")
        end
    end
    context "simulating 'username' execution" do
        it "creates the user account" do
            use_fixture '0082-username-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include('useradd -c "Adélie User" -m -U chris')
            expect(last_command_started.stdout).to include('useradd -c "Adélie User" -m -U kayla')
            expect(last_command_started.stdout).to include('useradd -c "Adélie User" -m -U meg')
            expect(last_command_started.stdout).to include('useradd -c "Adélie User" -m -U steph')
            expect(last_command_started.stdout).to include('useradd -c "Adélie User" -m -U amanda')
        end
    end
    context "simulating 'useralias' execution" do
        it "sets aliases on all named accounts" do
            use_fixture '0087-useralias-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include('usermod -c "Christopher" chris')
            expect(last_command_started.stdout).to include('usermod -c "Kayla" kayla')
            expect(last_command_started.stdout).to include('usermod -c "Meaghan" meg')
            expect(last_command_started.stdout).to include('usermod -c "Stephanie" steph')
            expect(last_command_started.stdout).to include('usermod -c "Amanda Jane" amanda')
        end
    end
    context "simulating 'userpw' execution" do
        it "sets the user's passphrase" do
            use_fixture '0091-userpw-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("usermod -p '$6$UZJm/vBmVgyIdMZr$ppKEulz/HY0/e7RcXXujQbcqDXkUYgIqNEVPQJO6.le9kUpz8GvvRezY3ifqUUEwjhSo9tTOMG7lhqjn8gGpH0' awilfox")
        end
    end
    context "simulating 'usergroups' execution" do
        it "sets groups" do
            use_fixture '0104-usergroups-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include('usermod -aG ')
        end
    end
    context "simulating 'usericon' execution" do
        it "creates the dir if needed" do
            use_fixture '0098-usericon-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("mkdir -p /target/var/lib/AccountsService/icons")
        end
        it "downloads remote icons" do
            use_fixture '0102-usericon-protocols.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("curl -LO /target/var/lib/AccountsService/icons/chris http://www.adelielinux.org/")
        end
        it "copies the correct icon" do
            use_fixture '0098-usericon-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("cp /usr/share/user-manager/avatars/circles/Cat.png /target/var/lib/AccountsService/icons/awilfox")
        end
    end
end
