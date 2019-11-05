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
    end
    context "simulating 'fs' execution" do
        it "creates ext4 filesystems correctly" do
            use_fixture '0179-fs-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("mkfs.ext4 -q -z /tmp/undo-sdb1 /dev/sdb1")
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
    end
    context "simulating 'nameserver' execution" do
        it "configures nameservers correctly" do
            use_fixture '0183-nameserver-basic.installfile'
            run_simulate
            expect(last_command_started.stdout).to include("printf 'nameserver %s\n' 172.16.1.1 >>/target/etc/resolv.conf")
        end
    end
end
