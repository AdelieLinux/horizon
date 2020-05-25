require 'spec_helper'

RSpec.describe 'HorizonScript Owner Print Utility', :type => :aruba do
    before { if !system("command -v hscript-printowner") then skip("Utility not built") end }
    context "argument passing" do
        it "requires an path to be specified" do
            run_command 'hscript-printowner'
            expect(last_command_started).to have_output(/requires a path or file/)
        end
        it "requires exactly one path to be specified" do
            run_command 'hscript-printowner a b c'
            expect(last_command_started).to have_output(/requires a path or file/)
        end
    end
    it "handles inaccessible paths correctly" do
        run_command 'hscript-printowner /root/foo'
        expect(last_command_started.stderr).to include("Permission denied")
    end
    it "handles nonexistent paths correctly" do
        run_command 'hscript-printowner /nonexistent'
        expect(last_command_started.stderr).to include("No such file or directory")
    end
    it "handles normal paths correctly" do
        run_command 'hscript-printowner /'
        expect(last_command_started.stdout).to eq("0\n")
    end
end
