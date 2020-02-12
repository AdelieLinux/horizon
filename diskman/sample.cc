#include "diskman/diskman.hh"
#include <iostream>

using Horizon::DiskMan::Disk;
std::string label_name(Disk::Label label) { switch(label) { case Disk::APM: return "APM"; case Disk::MBR: return "MBR"; case Disk::GPT: return "GPT"; } return "Unknown"; }

int main(void) {
	Horizon::DiskMan::DiskMan diskMan;
	auto disks = diskMan.find_disks(true, true, false);
	std::cout << "Found " << std::to_string(disks.size()) << " disk(s):" << std::endl;
	for(auto &disk : disks) {
		std::cout << "==========================================" << std::endl;
		std::cout << "Disk: " << disk.name() << " (" << disk.model() << ")";
		std::cout << " at " << disk.dev_path() << std::endl;
		std::cout << std::to_string(disk.total_size()) << " MiB total";
		std::cout << " (" << std::to_string(disk.free_space()) << " MiB free; ";
		std::cout << std::to_string(disk.contiguous_block()) << " MiB largest contiguous block)" << std::endl;
		std::cout << "Label: " << (disk.has_label() ? label_name(disk.label()) : "No") << std::endl;

		if(disk.has_label()) {
			std::cout << std::endl << "\tPartitions:" << std::endl;
			for(auto &part : disk.partitions()) {
				std::cout << "\t\t" << part.size() / 1048576 << " MiB (" << part.fstype();
				if(part.label().size() > 0) std::cout << ": " << part.label();
				std::cout << ")" << std::endl;
			}
		}
	}
	std::cout << "==========================================" << std::endl;
	return 0;
}
