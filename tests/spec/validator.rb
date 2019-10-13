require 'aruba/rspec'

IFILE_PATH = 'installfile'

def run_validate(extra = '')
    run_command 'hscript-validate ' + IFILE_PATH + extra
end

def use_fixture(fixture)
    copy '%/' + fixture, IFILE_PATH
end

PARSER_SUCCESS = /parser: 0 error\(s\), 0 warning\(s\)/
VALIDATOR_SUCCESS = /validator: 0 failure\(s\)/

RSpec.describe 'HorizonScript Validation Utility', :type => :aruba do
    context "argument passing" do
        it "requires an installfile to be specified" do
            run_command 'hscript-validate'
            expect(last_command_started).to have_output(/usage/)
        end
        it "doesn't output ANSI colours when instructed not to" do
            run_command 'hscript-validate foo -n'
            expect(last_command_started).to_not have_output(/\033/)
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
            context "for 'network' key" do
                # Runner.Validate.network.
                it "fails with an invalid 'network' value" do
                    use_fixture '0011-invalid-network.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*network.*/)
                end
            end
            # Runner.Validate.hostname.
            context "for 'hostname' key" do
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
                # Runner.Validate.hostname.
                it "with dot at end of domain" do
                    use_fixture '0032-hostname-ends-with-dot.installfile'
                    run_validate
                    expect(last_command_started).to_not have_output(/error: .*hostname.*/)
                end
            end
            # Runner.Validate.rootpw.
            # Runner.Validate.rootpw.Crypt.
            it "fails with an invalid 'rootpw' value" do
                use_fixture '0013-invalid-rootpw.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*rootpw.*/)
            end
            context "for 'mount' key" do
                # Runner.Validate.mount.
                it "fails with an invalid value" do
                    use_fixture '0014-invalid-mount.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*mount.*/)
                end
                # Runner.Validate.mount.Validity.
                it "fails with too many values in 'mount' tuple" do
                    use_fixture '0029-mount-too-many.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*mount.*elements/)
                end
                # Runner.Validate.mount.Validity.
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
                # Runner.Validate.mount.Root.
                it "fails without a root 'mount' key" do
                    use_fixture '0031-mount-nonroot.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*mount.*root/)
                end
            end
            context "for 'netaddress' key" do
                # Runner.Validate.network. / Runner.Validate.netaddress.
                it "requires 'netaddress' when 'network' is true" do
                    use_fixture '0033-network-without-netaddress.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*/)
                end
                it "allows 'netaddress' when 'network' is false" do
                    use_fixture '0034-nonetwork-with-netaddress.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "succeeds with simple 'netaddress' (DHCP on eth0)" do
                    use_fixture '0035-network-with-netaddress.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                # Runner.Validate.netaddress.Validity.
                it "requires 'netaddress' to have at least two elements" do
                    use_fixture '0036-netaddress-too-few.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*require/)
                end
                # Runner.Validate.netaddress.Validity.Type.
                it "fails on invalid address type" do
                    use_fixture '0037-netaddress-invalid-type.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*type/)
                end
                # Runner.Validate.netaddress.Validity.DHCP.
                it "fails on extraneous elements in DHCP mode" do
                    use_fixture '0038-netaddress-invalid-dhcp.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*dhcp/)
                end
                # Runner.Validate.netaddress.Validity.DHCP.
                it "fails on extraneous elements in SLAAC mode" do
                    use_fixture '0058-netaddress-invalid-slaac.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*slaac/)
                end
                # Runner.Validate.netaddress.Validity.Static.
                it "fails on extraneous elements in static mode" do
                    use_fixture '0039-netaddress-static-too-many.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*too many/)
                end
                # Runner.Validate.netaddress.Validity.Static.
                it "fails on too few elements in static mode" do
                    use_fixture '0040-netaddress-static-too-few.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*require/)
                end
                # Runner.Validate.netaddress.Validity.Address.
                it "succeeds with valid IPv4 address specification" do
                    use_fixture '0041-netaddress-valid-static4.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                # Runner.Validate.netaddress.Validity.Address.
                it "succeeds with valid IPv6 address specification" do
                    use_fixture '0042-netaddress-valid-static6.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                # Runner.Validate.netaddress.Validity.Address.
                it "fails with invalid IPv4 address specification" do
                    use_fixture '0043-netaddress-invalid-static4.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*address/)
                end
                # Runner.Validate.netaddress.Validity.Address.
                it "fails with invalid IPv6 address specification" do
                    use_fixture '0044-netaddress-invalid-static6.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*address/)
                end
                # Runner.Validate.netaddress.Validity.Address.
                it "fails with invalid address" do
                    use_fixture '0059-netaddress-invalid-address.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*address/)
                end
                # Runner.Validate.netaddress.Validity.Mask.
                it "fails with invalid IPv4 prefix length" do
                    use_fixture '0045-netaddress-invalid-prefix4.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*/)
                end
                # Runner.Validate.netaddress.Validity.Mask.
                it "fails with invalid IPv6 prefix length" do
                    use_fixture '0046-netaddress-invalid-prefix6.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*prefix/)
                end
                # Runner.Validate.netaddress.Validity.Mask.
                it "fails with non-numeric IPv4 prefix length" do
                    use_fixture '0060-netaddress-invalid-prefix4.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*prefix/)
                end
                # Runner.Validate.netaddress.Validity.Mask.
                it "fails with non-numeric IPv6 prefix length" do
                    use_fixture '0061-netaddress-invalid-prefix6.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*prefix/)
                end
                # Runner.Validate.netaddress.Validity.Mask.
                it "fails with invalid IPv4 network mask" do
                    use_fixture '0047-netaddress-invalid-mask.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*/)
                end
                # Runner.Validate.netaddress.Validity.Gateway.
                it "succeeds with valid IPv4 gateway" do
                    use_fixture '0048-netaddress-gateway4.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                # Runner.Validate.netaddress.Validity.Gateway.
                it "succeeds with valid IPv6 gateway" do
                    use_fixture '0049-netaddress-gateway6.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                # Runner.Validate.netaddress.Validity.Gateway.
                it "fails with invalid IPv4 gateway" do
                    use_fixture '0050-netaddress-bad-gateway4.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*gateway/)
                end
                # Runner.Validate.netaddress.Validity.Gateway.
                it "fails with invalid IPv6 gateway" do
                    use_fixture '0051-netaddress-bad-gateway6.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*gateway/)
                end
                # Runner.Validate.netaddress.Validity.Gateway.
                it "fails with mismatched IPv4/v6 gateway" do
                    use_fixture '0052-netaddress-bad-gateway46.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*gateway/)
                end
                # Runner.Validate.netaddress.Validity.Gateway.
                it "fails with mismatched IPv6/v4 gateway" do
                    use_fixture '0053-netaddress-bad-gateway64.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*gateway/)
                end
                # Runner.Validate.netaddress.Validity.Count.
                it "fails with too many addresses" do
                    use_fixture '0054-huge-netaddress.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*addresses/)
                end
            end
            context "for 'repository' key" do
                it "succeeds with basic repositories" do
                    use_fixture '0055-repository-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "fails with invalid repository URL" do
                    use_fixture '0056-repository-invalid.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*repository/)
                end
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
