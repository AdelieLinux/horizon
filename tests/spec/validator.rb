require 'aruba/rspec'

IFILE_PATH = 'installfile'

def run_validate(extra = '')
    run_command 'hscript-validate ' + IFILE_PATH + extra
end

def use_fixture(fixture)
    copy '%/' + fixture, IFILE_PATH
end

SUCCESS_OUTPUT = '0 error(s), 0 warning(s)'

RSpec.describe 'HorizonScript Validation Utility', :type => :aruba do
    context "argument passing" do
        it "requires an installfile to be specified" do
            run_command 'hscript-validate'
            expect(last_command_started).to have_output(/usage/)
        end
        it "doesn't output ANSI colours when redirected" do
            run_command 'hscript-validate foo 2>/dev/null'
            expect(last_command_started).to_not have_output(/\033/)
        end
    end
    context "on invalid keys" do
        before(:each) { use_fixture '0016-invalid-key.installfile' }
        it "warns on invalid keys by default" do
            run_validate
            expect(last_command_started).to have_output(/warning: .*chat.* not defined/)
        end
        it "errors on invalid keys in strict mode" do
            run_validate ' --strict'
            expect(last_command_started).to have_output(/error: .*chat.* not defined/)
        end
    end
    context "parsing" do
        it "successfully reads a basic installfile" do
            use_fixture '0001-basic.installfile'
            run_validate
            expect(last_command_started).to have_output(SUCCESS_OUTPUT)
        end
        it "handles comments" do
            use_fixture '0002-basic-commented.installfile'
            run_validate
            expect(last_command_started).to have_output(SUCCESS_OUTPUT)
        end
        it "handles blank lines and indentation" do
            use_fixture '0003-basic-whitespace.installfile'
            run_validate
            expect(last_command_started).to have_output(SUCCESS_OUTPUT)
        end
        it "requires keys to have values" do
            use_fixture '0015-keys-without-values.installfile'
            run_validate ' --keep-going'
            expect(last_command_started).to have_output(/5 error(s)/)
        end
        it "fails on lines over maximum line length" do
            use_fixture '0017-line-too-long.installfile'
            run_validate
            expect(last_command_started).to have_output(/error: .*length/)
        end
        context "required keys" do
            it "fails without a 'network' key" do
                use_fixture '0006-no-network.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*network.*/)
            end
            it "fails without a 'hostname' key" do
                use_fixture '0007-no-hostname.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*hostname.*/)
            end
            it "fails without a 'pkginstall' key" do
                use_fixture '0008-no-pkginstall.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*pkginstall.*/)
            end
            it "fails without a 'rootpw' key" do
                use_fixture '0009-no-rootpw.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*rootpw.*/)
            end
            it "fails without a 'mount' key" do
                use_fixture '0010-no-mount.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*mount.*/)
            end
        end
        context "values" do
            it "fails with an invalid 'network' value" do
                use_fixture '0011-invalid-network.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*network.*/)
            end
            it "fails with an invalid 'hostname' value" do
                use_fixture '0012-invalid-hostname.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*hostname.*/)
            end
            it "fails with an invalid 'rootpw' value" do
                use_fixture '0013-invalid-rootpw.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*rootpw.*/)
            end
            it "fails with an invalid 'mount' value" do
                use_fixture '0014-invalid-mount.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*mount.*/)
            end
        end
        context "unique keys" do
            it "fails with a duplicate 'network' key" do
                use_fixture '0018-duplicate-network.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*duplicate.*network/)
            end
            it "fails with a duplicate 'hostname' key" do
                use_fixture '0019-duplicate-hostname.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*duplicate.*hostname/)
            end
            it "fails with a duplicate 'rootpw' key" do
                use_fixture '0020-duplicate-rootpw.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*duplicate.*rootpw/)
            end
        end
    end
end
