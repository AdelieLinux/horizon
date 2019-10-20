require 'aruba/rspec'

IFILE_PATH = 'installfile'

def run_simulate(extra = '')
    run_command 'hscript-simulate ' + IFILE_PATH + extra
end

def use_fixture(fixture)
    copy '%/' + fixture, IFILE_PATH
end

Aruba.configure do |config|
    config.fixtures_directory = "fixtures" if respond_to? :fixtures_directory=
    config.fixtures_directories = %w(fixtures) if respond_to? :fixtures_directories=
    config.keep_ansi = true if respond_to? :keep_ansi=
    config.remove_ansi_escape_sequences = false if respond_to? :remove_ansi_escape_sequences=
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
end
