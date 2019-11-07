require 'spec_helper'

def run_validate(extra = '')
    run_command 'hscript-validate ' + IFILE_PATH + extra
end

PARSER_SUCCESS = /parser: 0 error\(s\), 0 warning\(s\)/
VALIDATOR_SUCCESS = /validator: 0 failure\(s\)/

RSpec.describe 'HorizonScript validation', :type => :aruba do
    context "Utility argument passing" do
        it "requires an installfile to be specified" do
            run_command 'hscript-validate'
            expect(last_command_started).to have_output(/usage/)
        end
        it "accepts -i flag" do
            run_command 'hscript-validate foo -i'
            expect(last_command_started).to_not have_output(/usage/)
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
    context "with invalid keys" do
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
            expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
        end
        # HorizonScript Specification, ch 3.
        it "handles comments" do
            use_fixture '0002-basic-commented.installfile'
            run_validate
            expect(last_command_started).to have_output(PARSER_SUCCESS)
            expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
        end
        # HorizonScript Specification, ch 3.
        it "handles blank lines and indentation" do
            use_fixture '0003-basic-whitespace.installfile'
            run_validate
            expect(last_command_started).to have_output(PARSER_SUCCESS)
            expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
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
            context "for 'language' key" do
                # Runner.Validate.language
                # Runner.Validate.language.Format
                it "supports a simple language" do
                    use_fixture '0115-language-simple.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "supports a language_territory value" do
                    use_fixture '0116-language-country.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "supports the UTF-8 codeset" do
                    use_fixture '0117-language-codeset.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "supports the special case C.UTF-8" do
                    use_fixture '0118-language-c-exception.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "requires a valid language" do
                    use_fixture '0119-language-invalid.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*language.*invalid/)
                end
                it "requires the UTF-8 codeset" do
                    use_fixture '0120-language-iso.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*language.*codeset/)
                end
                it "requires a valid two-char language" do
                    use_fixture '0199-language-invalid2.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*language.*invalid/)
                end
            end
            context "for 'nameserver' key" do
                it "succeeds with IPv4 and IPv6 addresses" do
                    use_fixture '0183-nameserver-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "fails with an invalid value" do
                    use_fixture '0184-nameserver-invalid.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*nameserver.*expected/)
                end
                it "fails with brackets around IPv6 addresses" do
                    use_fixture '0185-nameserver-brackets.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/nameserver.*brackets/)
                end
                it "warns with more than MAXNS nameservers" do
                    use_fixture '0201-nameserver-max.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/warning: .*nameserver.*more/)
                end
                it "fails with an invalid IPv4 address" do
                    use_fixture '0202-nameserver-bad4.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*nameserver.*valid IPv4/)
                end
                it "fails with an invalid IPv6 address" do
                    use_fixture '0203-nameserver-bad6.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*nameserver.*valid IPv6/)
                end
            end
            context "for 'firmware' key" do
                it "always supports 'false' value" do
                    use_fixture '0112-firmware-false.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "requires 'false' if built without support" do
                    use_fixture '0111-firmware-true.installfile'
                    run_validate
                    skip "This build supports firmware" if last_command_started.stdout =~ /supports non-free/
                    expect(last_command_started).to have_output(/error: .*firmware/)
                end
                it "supports 'true' if built with support" do
                    use_fixture '0111-firmware-true.installfile'
                    run_validate
                    skip "This build does not support firmware" if last_command_started.stdout !~ /supports non-free/
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "requires a boolean value" do
                    use_fixture '0113-firmware-invalid.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*firmware.*value/)
                end
            end
            context "for 'timezone' key" do
                # Runner.Validate.timezone.
                it "fails with an invalid value" do
                    use_fixture '0132-timezone-invalid.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*timezone.*invalid/)
                end
                # Runner.Validate.timezone.
                it "fails with a maliciously invalid value" do
                    use_fixture '0133-timezone-malicious.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*timezone.*invalid/)
                end
                # Runner.Validate.timezone.zoneinfo.
                it "succeeds with simple value" do
                    use_fixture '0131-timezone-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                # Runner.Validate.timezone.zoneinfo.
                it "succeeds with Locality/Zone value" do
                    use_fixture '0134-timezone-subtz.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
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
                it "fails with invalid interface name" do
                    use_fixture '0130-netaddress-invalid-iface.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netaddress.*interface/)
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
            context "for 'netssid' key" do
                it "succeeds with simple SSID, no security" do
                    use_fixture '0062-netssid-simple-none.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "succeeds with simple SSID, WEP security" do
                    use_fixture '0063-netssid-simple-wep.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "succeeds with simple SSID, WPA security" do
                    use_fixture '0064-netssid-simple-wpa.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "succeeds with complex SSID, no security" do
                    use_fixture '0065-netssid-spaces-none.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "succeeds with complex SSID, WEP security" do
                    use_fixture '0066-netssid-spaces-wep.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "succeeds with complex SSID, WPA security" do
                    use_fixture '0067-netssid-spaces-wpa.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "fails with invalid interface name" do
                    use_fixture '0068-netssid-invalid-iface.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netssid.*interface/)
                end
                it "fails with raw / unquoted SSID" do
                    use_fixture '0069-netssid-unquoted.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netssid.*quote/)
                end
                it "fails with SSID without terminating quote" do
                    use_fixture '0070-netssid-syntax-error.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netssid.*unterminated/)
                end
                it "fails with missing passphrase" do
                    use_fixture '0071-netssid-missing-pw.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netssid.*expected/)
                end
                it "fails with missing SSID" do
                    use_fixture '0072-netssid-missing-ssid.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netssid.*expected/)
                end
                it "fails with invalid security type" do
                    use_fixture '0073-netssid-invalid-type.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*netssid.*security/)
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
            context "for 'signingkey' key" do
                it "succeeds with secure key URL" do
                    use_fixture '0186-signingkey-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "succeeds with local key URL" do
                    use_fixture '0187-signingkey-local.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "fails with insecure key URL" do
                    use_fixture '0188-signingkey-insec.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*signingkey/)
                end
            end
            context "for 'pkginstall' key" do
                it "warns when a package is listed twice in the same line" do
                    use_fixture '0216-pkginstall-dup-single.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/warning: .*pkginstall.*already/)
                end
                it "warns when a package is listed twice in the same script" do
                    use_fixture '0217-pkginstall-dup-multi.installfile'
                    run_validate ' -s'
                    expect(last_command_started).to have_output(/warning: .*pkginstall.*already/)
                end
            end
            context "for 'diskid' key" do
                it "succeeds with basic disk identification" do
                    use_fixture '0076-diskid-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "requires an identification string" do
                    use_fixture '0077-diskid-missing-id.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*diskid.*expected/)
                end
                it "succeeds with present disk" do
                    use_fixture '0078-diskid-diskid-loop0.installfile'
                    run_validate ' -i'
                    skip "This build does not support this test" if last_command_started.stdout =~ /runtime environment only/
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "fails with non-present disk" do
                    use_fixture '0079-diskid-enoent.installfile'
                    run_validate ' -i'
                    skip "This build does not support this test" if last_command_started.stdout =~ /runtime environment only/
                    expect(last_command_started).to have_output(/No such file or directory/)
                end
                it "fails with non-disk file" do
                    use_fixture '0080-diskid-tmp.installfile'
                    run_validate ' -i'
                    skip "This build does not support this test" if last_command_started.stdout =~ /runtime environment only/
                    expect(last_command_started).to have_output(/error: .*diskid.*block/)
                end
                it "fails with a duplicate identification device" do
                    use_fixture '0081-diskid-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*diskid.*already/)
                end
            end
            context "for 'disklabel' key" do
                it "requires a label type" do
                    use_fixture '0129-disklabel-without-label.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*disklabel.*expected/)
                end
                it "succeeds with a APM disk label" do
                    use_fixture '0122-disklabel-apm.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "succeeds with a MBR disk label" do
                    use_fixture '0123-disklabel-mbr.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "succeeds with a GPT disk label" do
                    use_fixture '0124-disklabel-gpt.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "fails with a duplicate disk" do
                    use_fixture '0125-disklabel-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*disklabel.*already/)
                end
                it "succeeds with present disk" do
                    use_fixture '0126-disklabel-loop0.installfile'
                    run_validate ' -i'
                    skip "This build does not support this test" if last_command_started.stdout =~ /runtime environment only/
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "fails with non-present disk" do
                    use_fixture '0127-disklabel-enoent.installfile'
                    run_validate ' -i'
                    skip "This build does not support this test" if last_command_started.stdout =~ /runtime environment only/
                    expect(last_command_started).to have_output(/error: .*disklabel.*block/)
                end
                it "requires a valid label type" do
                    use_fixture '0128-disklabel-invalid-type.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*disklabel.*type/)
                end
            end
            context "for 'partition' key" do
                it "succeeds with a typical value" do
                    use_fixture '0135-partition-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                context "size value" do
                    it "supports bytes" do
                        use_fixture '0136-partition-size-bytes.installfile'
                        run_validate
                        expect(last_command_started).to have_output(PARSER_SUCCESS)
                        expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                    end
                    it "supports kilobytes (K)" do
                        use_fixture '0137-partition-size-kbytes.installfile'
                        run_validate
                        expect(last_command_started).to have_output(PARSER_SUCCESS)
                        expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                    end
                    it "supports megabytes (M)" do
                        use_fixture '0138-partition-size-mbytes.installfile'
                        run_validate
                        expect(last_command_started).to have_output(PARSER_SUCCESS)
                        expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                    end
                    it "supports gigabytes (G)" do
                        use_fixture '0139-partition-size-gbytes.installfile'
                        run_validate
                        expect(last_command_started).to have_output(PARSER_SUCCESS)
                        expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                    end
                    it "supports terabytes (T)" do
                        use_fixture '0140-partition-size-tbytes.installfile'
                        run_validate
                        expect(last_command_started).to have_output(PARSER_SUCCESS)
                        expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                    end
                    it "does not support exabytes (E)" do
                        use_fixture '0141-partition-size-ebytes.installfile'
                        run_validate
                        expect(last_command_started).to have_output(/error: .*suffix/)
                    end
                    it "supports percentages" do
                        use_fixture '0142-partition-size-percent.installfile'
                        run_validate
                        expect(last_command_started).to have_output(PARSER_SUCCESS)
                        expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                    end
                    it "does not support percentages over 100" do
                        use_fixture '0143-partition-size-percent-overflow.installfile'
                        run_validate
                        expect(last_command_started).to have_output(/error: .*too large/)
                    end
                    it "supports the literal value 'fill'" do
                        use_fixture '0144-partition-size-fill.installfile'
                        run_validate
                        expect(last_command_started).to have_output(PARSER_SUCCESS)
                        expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                    end
                    it "does not support non-numeric values" do
                        use_fixture '0145-partition-size-nonnumeric.installfile'
                        run_validate
                        expect(last_command_started).to have_output(/error: .*number/)
                    end
                    it "does not support incorrect suffixes" do
                        use_fixture '0146-partition-size-bad-suffix.installfile'
                        run_validate
                        expect(last_command_started).to have_output(/error: .*suffix/)
                    end
                    it "correctly handles byte overflow" do
                        use_fixture '0147-partition-size-overflow.installfile'
                        run_validate
                        expect(last_command_started).to have_output(/error: .*too large/)
                    end
                    it "correctly handles kilobyte overflow (K)" do
                        use_fixture '0148-partition-size-overflow-k.installfile'
                        run_validate
                        expect(last_command_started).to have_output(/error: .*too large/)
                    end
                    it "correctly handles megabyte overflow (M)" do
                        use_fixture '0149-partition-size-overflow-m.installfile'
                        run_validate
                        expect(last_command_started).to have_output(/error: .*too large/)
                    end
                    it "correctly handles gigabyte overflow (G)" do
                        use_fixture '0150-partition-size-overflow-g.installfile'
                        run_validate
                        expect(last_command_started).to have_output(/error: .*too large/)
                    end
                    it "correctly handles terabyte overflow (T)" do
                        use_fixture '0151-partition-size-overflow-t.installfile'
                        run_validate
                        expect(last_command_started).to have_output(/error: .*too large/)
                    end
                    it "supports mixed-case 'fill'" do
                        use_fixture '0162-partition-size-case.installfile'
                        run_validate
                        expect(last_command_started).to have_output(PARSER_SUCCESS)
                        expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                    end
                end
                context "type code" do
                    it "handles 'boot'" do
                        use_fixture '0152-partition-type-boot.installfile'
                        run_validate
                        expect(last_command_started).to have_output(PARSER_SUCCESS)
                        expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                    end
                    it "handles 'esp'" do
                        use_fixture '0153-partition-type-esp.installfile'
                        run_validate
                        expect(last_command_started).to have_output(PARSER_SUCCESS)
                        expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                    end
                    it "handles no value" do
                        use_fixture '0154-partition-type-none.installfile'
                        run_validate
                        expect(last_command_started).to have_output(PARSER_SUCCESS)
                        expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                    end
                    it "errors on invalid type code" do
                        use_fixture '0155-partition-type-invalid.installfile'
                        run_validate
                        expect(last_command_started).to have_output(/error: .*partition.*invalid/)
                    end
                end
                it "errors on non-numeric partition number" do
                    use_fixture '0156-partition-num-nonnumeric.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*partition.*number/)
                end
                it "requires a block device" do
                    use_fixture '0157-partition-nonblock.installfile'
                    run_validate ' -i'
                    skip "This build does not support this test" if last_command_started.stdout =~ /runtime environment only/
                    expect(last_command_started).to have_output(/error: .*partition.*block/)
                end
                it "requires an absolute device path" do
                    use_fixture '0158-partition-nondev.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*partition.*device/)
                end
                it "requires partition numbers to be unique per block device" do
                    use_fixture '0159-partition-num-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*partition.*already exists/)
                end
                it "requires a partition number" do
                    use_fixture '0160-partition-missing-num.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*partition.*expected/)
                end
                it "requires a size" do
                    use_fixture '0161-partition-missing-size.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*partition.*size/)
                end
            end
            context "for 'lvm_pv' key" do
                it "succeeds with normal value" do
                    use_fixture '0163-lvmpv-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "fails with a non-absolute block device path" do
                    use_fixture '0164-lvmpv-invalid.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_pv.*path/)
                end
                it "fails with duplicate physical volumes" do
                    use_fixture '0165-lvmpv-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_pv.*exists/)
                end
            end
            context "for 'lvm_vg' key" do
                it "succeeds with a normal value" do
                    use_fixture '0166-lvmvg-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "fails with an invalid PV path" do
                    use_fixture '0167-lvmvg-invalid.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_vg.*path/)
                end
                it "fails with a duplicate volume group name" do
                    use_fixture '0168-lvmvg-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_vg.*exists/)
                end
                it "fails with a dash as the first character of the VG name" do
                    use_fixture '0169-lvmvg-dash.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_vg.*name/)
                end
                it "requires a volume group name" do
                    use_fixture '0170-lvmvg-without-vg.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_vg.*expected/)
                end
                it "fails with a dot as the VG name" do
                    use_fixture '0207-lvmvg-dot.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_vg.*name/)
                end
                it "succeeds with a dot in the VG name" do
                    use_fixture '0208-lvmvg-valid-dot.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "warns when a PV isn't created" do
                    use_fixture '0214-lvmvg-without-pv.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/warning: .*lvm_vg.*physical volume/)
                end
            end
            context "for 'lvm_lv' key" do
                it "succeeds with a simple value" do
                    use_fixture '0171-lvmlv-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "requires a valid volume group name" do
                    use_fixture '0172-lvmlv-badvg.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_lv.*name/)
                end
                it "requires a valid logical volume name" do
                    use_fixture '0173-lvmlv-badlv.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_lv.*name/)
                end
                it "requires a valid size" do
                    use_fixture '0174-lvmlv-bad-size.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_lv.*size/)
                end
                it "requires a size to be specified" do
                    use_fixture '0175-lvmlv-invalid.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_lv.*expected/)
                end
                it "requires a unique VG/LV name pair" do
                    use_fixture '0176-lvmlv-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_lv.*already/)
                end
                it "requires the volume group to be present" do
                    use_fixture '0177-lvmlv-no-vg.installfile'
                    run_validate ' -i'
                    skip "This build does not support this test" if last_command_started.stdout =~ /runtime environment only/
                    expect(last_command_started).to have_output(/error: .*lvm_lv.*volume group/)
                end
                it "fails if the logical volume name starts with a dash" do
                    use_fixture '0209-lvmlv-dash.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_lv.*name/)
                end
                it "fails if the logical volume name is reserved" do
                    use_fixture '0210-lvmlv-snapshot.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_lv.*name/)
                    use_fixture '0211-lvmlv-pvmove.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*lvm_lv.*name/)
                end
            end
            context "for 'encrypt' key" do
                it "succeeds with a simple value" do
                    use_fixture '0189-encrypt-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "fails with a duplicate block device" do
                    use_fixture '0190-encrypt-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*encrypt.*already/)
                end
                it "fails with an invalid block device" do
                    use_fixture '0191-encrypt-invalid.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*encrypt.*expected/)
                end
                it "succeeds with a passphrase" do
                    use_fixture '0192-encrypt-pw.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
            end
            context "for 'fs' key" do
                it "succeeds with a simple value" do
                    use_fixture '0179-fs-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "requires a filesystem type" do
                    use_fixture '0180-fs-without-fs.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*fs.*expected/)
                end
                it "requires a valid absolute block device path" do
                    use_fixture '0181-fs-invalid-dev.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*fs.*device/)
                end
                it "requires a valid filesystem type" do
                    use_fixture '0182-fs-invalid-type.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*fs.*type/)
                end
                it "fails with a duplicate block device" do
                    use_fixture '0206-fs-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*fs.*already/)
                end
            end
            context "for 'keymap' key" do
                it "succeeds with a simple value" do
                    use_fixture '0178-keymap-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "fails with an invalid value" do
                    use_fixture '0198-keymap-invalid.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*keymap.*invalid/)
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
            # Runner.Validate.firmware.
            it "fails with a duplicate 'firmware' key" do
                use_fixture '0114-firmware-duplicate.installfile'
                run_validate
                skip "This build does not support firmware" if last_command_started.stdout !~ /supports non-free/
                expect(last_command_started).to have_output(/error: .*duplicate.*firmware/)
            end
            it "fails with a duplicate 'language' key" do
                use_fixture '0121-language-duplicate.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*duplicate.*language/)
            end
            it "fails with a duplicate 'keymap' key" do
                use_fixture '0204-keymap-duplicate.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*duplicate.*keymap/)
            end
            it "fails with a duplicate 'timezone' key" do
                use_fixture '0205-timezone-duplicate.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*duplicate.*timezone/)
            end
        end
        context "user account keys:" do
            context "'username'" do
                # Runner.Validate.username.
                it "succeeds with multiple usernames" do
                    use_fixture '0082-username-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                # Runner.Validate.username.Unique.
                it "fails with duplicate usernames" do
                    use_fixture '0083-username-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*duplicate.*username/)
                end
                # Runner.Validate.username.System.
                it "fails with a system username" do
                    use_fixture '0084-username-system.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*username.*system/)
                end
                # Runner.Validate.username.
                it "fails with more than 255 usernames" do
                    use_fixture '0085-all-the-username.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*username.*too many/)
                end
                it "fails with an invalid username" do
                    use_fixture '0086-username-invalid.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*username.*invalid/)
                end
            end
            context "'useralias'" do
                # Runner.Validate.useralias.
                it "succeeds with usernames with aliases" do
                    use_fixture '0087-useralias-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                # Runner.Validate.useralias.Validity.
                it "requires an alias to be provided" do
                    use_fixture '0088-useralias-without-alias.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*useralias.*required/)
                end
                # Runner.Validate.useralias.Name.
                it "fails with a username that wasn't given" do
                    use_fixture '0089-useralias-unknown-name.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*useralias.*name/)
                end
                # Runner.Validate.useralias.Unique.
                it "fails with a duplicated alias" do
                    use_fixture '0090-useralias-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*duplicate.*useralias/)
                end
            end
            context "'userpw'" do
                # Runner.Validate.userpw.
                it "succeeds with username/passphrase combinations" do
                    use_fixture '0091-userpw-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                # Runner.Validate.userpw.Validity.
                it "requires a passphrase to be provided" do
                    use_fixture '0092-userpw-without-pw.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*userpw.*required/)
                end
                # Runner.Validate.userpw.Name.
                it "fails with a username that wasn't given" do
                    use_fixture '0093-userpw-unknown-name.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*userpw.*name/)
                end
                # Runner.Validate.userpw.Unique.
                it "fails with more than one passphrase for an account" do
                    use_fixture '0094-userpw-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*duplicate.*userpw/)
                end
                # Runner.Validate.userpw.Crypt.
                it "fails with a plain-text password" do
                    use_fixture '0095-userpw-plaintext.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*userpw.*crypt/)
                end
                it "fails with an invalid encryption algorithm" do
                    use_fixture '0096-userpw-md5.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*userpw.*crypt/)
                end
                # Runner.Validate.userpw.None.
                it "warns when an account doesn't have a passphrase" do
                    use_fixture '0097-userpw-missing.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/warning: .*passphrase/)
                end
            end
            context "'usericon'" do
                # Runner.Validate.usericon.
                it "succeeds with a valid icon/account pair" do
                    use_fixture '0098-usericon-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                # Runner.Validate.usericon.Validity.
                it "requires a path to be provided" do
                    use_fixture '0099-usericon-without-icon.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*usericon.*required/)
                end
                # Runner.Validate.usericon.Name.
                it "fails with a username that wasn't given" do
                    use_fixture '0100-usericon-unknown-name.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*usericon.*name/)
                end
                # Runner.Validate.usericon.Unique.
                it "fails with multiple icons associated with a single account" do
                    use_fixture '0101-usericon-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*duplicate.*usericon/)
                end
                # Runner.Validate.usericon.ValidPath.
                it "succeeds with all supported protocols" do
                    use_fixture '0102-usericon-protocols.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                it "fails with an unsupported protocol" do
                    use_fixture '0103-usericon-gopher.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*usericon.*URL/)
                end
            end
            context "'usergroups'" do
                # Runner.Validate.usergroups.
                it "succeeds with a valid account/group set" do
                    use_fixture '0104-usergroups-basic.installfile'
                    run_validate
                    expect(last_command_started).to have_output(PARSER_SUCCESS)
                    expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
                end
                # Runner.Validate.usergroups.Validity.
                it "requires at least one group to be provided" do
                    use_fixture '0105-usergroups-without-groups.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*usergroups.*required/)
                end
                it "doesn't allow a group name longer than maximum" do
                    use_fixture '0215-usergroups-length.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*usergroups.*exceeds maximum length/)
                end
                it "correctly errors when only a , is given" do
                    use_fixture '0106-usergroups-comma.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*usergroups.*invalid/)
                end
                # Runner.Validate.usergroups.Name.
                it "fails with a username that wasn't given" do
                    use_fixture '0107-usergroups-unknown-name.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*usergroups.*name/)
                end
                # Runner.Validate.usergroups.Count.
                it "fails with more than 16 groups for a single name" do
                    use_fixture '0108-jumbo-usergroups.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*usergroups.*16/)
                end
                # Runner.Validate.usergroups.Group.
                it "fails with an unknown group name" do
                    use_fixture '0109-usergroups-unknown.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*usergroups.*group/)
                end
                # Runner.Validate.usergroups.Unique.
                it "fails with the same group specified twice" do
                    use_fixture '0110-usergroups-duplicate.installfile'
                    run_validate
                    expect(last_command_started).to have_output(/error: .*usergroups.*duplicate/)
                end
            end
        end
        context "package specifications" do
            # no requirements for these, but I think obvious.
            it "works with all types of package atoms" do
                use_fixture '0022-all-kinds-of-atoms.installfile'
                run_validate
                expect(last_command_started).to have_output(PARSER_SUCCESS)
                expect(last_command_started).to have_output(VALIDATOR_SUCCESS)
            end
            it "does not accept invalid package atoms" do
                use_fixture '0023-pkginstall-invalid-modifier.installfile'
                run_validate
                expect(last_command_started).to have_output(/error: .*expected package.*/)
            end
        end
    end
end
