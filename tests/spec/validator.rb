require 'aruba/rspec'

IFILE_PATH = 'installfile'

def run_validate(extra = '')
    run_command 'hscript-validate ' + IFILE_PATH + extra
end

def use_fixture(fixture)
    copy '%/' + fixture, IFILE_PATH
end

PARSER_SUCCESS = /parser: 0 error\(s\), 0 warning\(s\)/

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
        # No requirement - but was noted in the original draft vision doc as
        # desireable because it allows future expansion while retaining some
        # compatibility.
        it "warns on invalid keys by default" do
            use_fixture '0016-invalid-key.installfile'
            run_validate
            expect(last_command_started).to have_output(/warning: .*chat.* not defined/)
        end
        it "errors on invalid keys in strict mode" do
            use_fixture '0016-invalid-key.installfile'
            run_validate ' --strict'
            expect(last_command_started).to have_output(/error: .*chat.* not defined/)
        end
    end
    context "parsing" do
        # obvious...
        it "successfully reads a basic installfile" do
            use_fixture '0001-basic.installfile'
            run_validate
            expect(last_command_started).to have_output(PARSER_SUCCESS)
        end
        # HorizonScript Specification, ch 3.
        it "handles comments" do
            use_fixture '0002-basic-commented.installfile'
            run_validate
            expect(last_command_started).to have_output(PARSER_SUCCESS)
        end
        # HorizonScript Specification, ch 3.
        it "handles blank lines and indentation" do
            use_fixture '0003-basic-whitespace.installfile'
            run_validate
            expect(last_command_started).to have_output(PARSER_SUCCESS)
        end
        it "requires keys to have values" do
            use_fixture '0015-keys-without-values.installfile'
            run_validate ' --keep-going'
            expect(last_command_started).to have_output(/parser: 2 error\(s\)/)
        end
        # XXX: no requirement.
        it "fails on lines over maximum line length" do
            use_fixture '0017-line-too-long.installfile'
            run_validate
            expect(last_command_started).to have_output(/error: .*length/)
        end
        context "required keys" do
            # Runner.Validate.Required.
            # Runner.Validate.network.
            it "fails without a 'network' key" do
                use_fixture '0006-no-network.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*network.*/)
            end
            # Runner.Validate.Required.
            # Runner.Validate.hostname.
            it "fails without a 'hostname' key" do
                use_fixture '0007-no-hostname.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*hostname.*/)
            end
            # Runner.Validate.Required.
            # Runner.Validate.pkginstall.
            it "fails without a 'pkginstall' key" do
                use_fixture '0008-no-pkginstall.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*pkginstall.*/)
            end
            # Runner.Validate.Required.
            # Runner.Validate.rootpw.
            it "fails without a 'rootpw' key" do
                use_fixture '0009-no-rootpw.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*rootpw.*/)
            end
            # Runner.Validate.Required.
            # Runner.Validate.mount.
            it "fails without a 'mount' key" do
                use_fixture '0010-no-mount.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*mount.*/)
            end
        end
        context "values" do
            # Runner.Validate.network.
            it "fails with an invalid 'network' value" do
                use_fixture '0011-invalid-network.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*network.*/)
            end
            # Runner.Validate.hostname.
            context "fails with an invalid 'hostname' value" do
                # Runner.Validate.hostname.Chars.
                it "with invalid characters" do
                    use_fixture '0012-invalid-hostname.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*hostname.*/)
                end
                # Runner.Validate.hostname.Begin.
                it "with non-alphabetical first character" do
                    use_fixture '0024-numeric-hostname.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*hostname.*/)
                end
                # Runner.Validate.hostname.Length
                it "with >320 characters" do
                    use_fixture '0025-jumbo-hostname.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*hostname.*/)
                end
                # Runner.Validate.hostname.PartLength
                it "with >64 characters in a single part" do
                    use_fixture '0026-jumbo-part-hostname.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*hostname.*/)
                end
            end
            # Runner.Validate.rootpw.
            # Runner.Validate.rootpw.Crypt.
            it "fails with an invalid 'rootpw' value" do
                use_fixture '0013-invalid-rootpw.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*rootpw.*/)
            end
            # Runner.Validate.mount.
            it "fails with an invalid 'mount' value" do
                use_fixture '0014-invalid-mount.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*mount.*/)
            end
            # Runner.Validate.mount.
            it "fails with too many values in 'mount' tuple" do
                use_fixture '0029-mount-too-many.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*mount.*elements/)
            end
            # Runner.Validate.mount.
            it "fails with too few values in 'mount' tuple" do
                use_fixture '0030-mount-too-few.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*mount.*elements/)
            end
            # Runner.Validate.mount.Block.
            it "fails with a 'mount' value that has no block device" do
                use_fixture '0027-mount-invalid-dev.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*mount.*device/)
            end
            # Runner.Validate.mount.Point.
            it "fails with a 'mount' value that has an invalid mountpoint" do
                use_fixture '0028-mount-non-absolute.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*mount.*path/)
            end
            # Runner.Validate.mount.Unique.
            it "fails with two root 'mount' keys" do
                use_fixture '0021-duplicate-root-mount.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*mount.*duplicate/)
            end
        end
        context "unique keys" do
            # Runner.Validate.network.
            it "fails with a duplicate 'network' key" do
                use_fixture '0018-duplicate-network.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*duplicate.*network/)
            end
            # Runner.Validate.hostname.
            it "fails with a duplicate 'hostname' key" do
                use_fixture '0019-duplicate-hostname.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*duplicate.*hostname/)
            end
            # Runner.Validate.rootpw.
            it "fails with a duplicate 'rootpw' key" do
                use_fixture '0020-duplicate-rootpw.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*duplicate.*rootpw/)
            end
        end
        context "package specifications" do
            # no requirements for these, but I think obvious.
            it "works with all types of package atoms" do
                use_fixture '0022-all-kinds-of-atoms.installfile'
                run_validate
                expect(last_command_started).to have_output(PARSER_SUCCESS)
            end
            it "does not accept invalid package atoms" do
                use_fixture '0023-pkginstall-invalid-modifier.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*expected package.*/)
            end
        end
    end
end
