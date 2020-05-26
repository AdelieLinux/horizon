/*
 * disk_lvm.cc - Implementation of the Key classes for LVM manipulation
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <algorithm>
#include <cstring>              /* strcmp */
#include <string>
#ifdef HAS_INSTALL_ENV
#   include <blkid/blkid.h>     /* blkid_get_tag_value */
#   include "util/filesystem.hh"
#endif /* HAS_INSTALL_ENV */
#include "disk.hh"
#include "util.hh"
#include "util/output.hh"

using namespace Horizon::Keys;

bool parse_size_string(const std::string &, uint64_t *, SizeType *);

Key *LVMPhysical::parseFromData(const std::string &data,
                                const ScriptLocation &pos, int *errors, int *,
                                const Script *script) {
    if(data.size() < 6 || data.substr(0, 5) != "/dev/") {
        if(errors) *errors += 1;
        output_error(pos, "lvm_pv: expected an absolute path to a device");
        return nullptr;
    }

    return new LVMPhysical(script, pos, data);
}

bool LVMPhysical::execute() const {
    output_info(pos, "lvm_pv: creating physical volume on " + _value);

    if(script->options().test(Simulate)) {
        std::cout << "pvcreate --force " << _value << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    const char *fstype = blkid_get_tag_value(nullptr, "TYPE", _value.c_str());
    if(fstype != nullptr && strcmp(fstype, "LVM2_member") == 0) {
        /* already a pv; skip */
        return true;
    }

    if(run_command("pvcreate", {"--force", _value}) != 0) {
        output_error(pos, "lvm_pv: failed to create physical volume", _value);
        return false;
    }
#endif /* HAS_INSTALL_ENV */
    return true;  /* LCOV_EXCL_LINE */
}


/*! Determine if a string is a valid LVM VG/LV name.
 * @param name      The name of which to test validity.
 * @returns true if the string is a valid name, false otherwise.
 * @note LVM LVs have additional restrictions; see is_valid_lvm_lv_name.
 */
bool is_valid_lvm_name(const std::string &name) {
    if(name[0] == '.' && (name.length() == 1 || name[1] == '.')) {
        /* . and .. are invalid */
        return false;
    }
    if(name[0] == '-') {
        /* VG nor LV may start with - */
        return false;
    }

    const std::string valid_syms("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+_.-");
    return (name.find_first_not_of(valid_syms) == std::string::npos);
}

/*! Determine if a string is a valid LVM LV name.
 * @param name      The name of which to test validity.
 * @returns true if the string is a valid LV name, false otherwise.
 */
bool is_valid_lvm_lv_name(const std::string &name) {
    if(!is_valid_lvm_name(name)) {
        /* Fail fast if we fail the general test. */
        return false;
    }

    if(name == "snapshot" || name == "pvmove") {
        /* Invalid full names. */
        return false;
    }

    if(name.find("_cdata") != std::string::npos ||
       name.find("_cmeta") != std::string::npos ||
       name.find("_corig") != std::string::npos ||
       name.find("_mlog") != std::string::npos ||
       name.find("_mimage") != std::string::npos ||
       name.find("_pmspare") != std::string::npos ||
       name.find("_rimage") != std::string::npos ||
       name.find("_rmeta") != std::string::npos ||
       name.find("_tdata") != std::string::npos ||
       name.find("_tmeta") != std::string::npos ||
       name.find("_vorigin") != std::string::npos) {
        /* Cannot occur anywhere in the name. */
        return false;
    }

    return true;
}


Key *LVMGroup::parseFromData(const std::string &data, const ScriptLocation &pos,
                             int *errors, int *, const Script *script) {
    std::string::size_type space = data.find_first_of(' ');
    if(space == std::string::npos || data.size() == space + 1) {
        if(errors) *errors += 1;
        output_error(pos, "lvm_vg: expected exactly two elements",
                     "syntax is lvm_vg [pv_block] [name-of-vg]");
        return nullptr;
    }

    const std::string pv(data.substr(0, space));
    const std::string name(data.substr(space + 1));

    if(pv.length() < 6 || pv.substr(0, 5) != "/dev/") {
        if(errors) *errors += 1;
        output_error(pos, "lvm_vg: expected absolute path to block device");
        return nullptr;
    }

    if(!is_valid_lvm_name(name)) {
        if(errors) *errors += 1;
        output_error(pos, "lvm_vg: invalid volume group name");
        return nullptr;
    }

    return new LVMGroup(script, pos, pv, name);
}

bool LVMGroup::validate() const {
    /* validation occurs during parsing */
    return true;
}

/* LCOV_EXCL_START */
bool LVMGroup::test_pv() const {
#ifdef HAS_INSTALL_ENV
    const char *fstype = blkid_get_tag_value(nullptr, "TYPE",
                                             this->pv().c_str());
    if(fstype == nullptr) {
        /* inconclusive */
        return true;
    }

    return (strcmp(fstype, "LVM2_member") == 0);
#else /* !HAS_INSTALL_ENV */
    return true;
#endif /* HAS_INSTALL_ENV */
}
/* LCOV_EXCL_STOP */

#ifdef HAS_INSTALL_ENV
/*! Determine if a named Volume Group currently exists on a LVM PV.
 * @param vg        The name of the Volume Group.
 * @param pv        The path to the LVM physical volume.
 * @param pos       The location of the line.
 * @param msgs      Whether or not to print messages.
 * @returns true if +vg+ appears on +pv+; false otherwise.
 */
bool does_vg_exist_on_pv(const std::string &vg, const std::string &pv,
                         const Horizon::ScriptLocation &pos, bool msgs) {
    bool success = false;
    const std::string pv_command("pvs --noheadings -o vg_name " + pv +
                                 " 2>/dev/null");

    FILE *pvs = popen(pv_command.c_str(), "r");
    if(pvs == nullptr) {
        if(msgs) output_error(pos, "lvm_vg: can't determine if vg is duplicate");
        return false;
    }

    char *buf = nullptr;
    size_t buf_size = 0;
    ssize_t read_bytes = getline(&buf, &buf_size, pvs);

    pclose(pvs);

    /* size must match *exactly* or else we could have a short compare succeed
     * i.e. on-disk VG "Group", our VG "GroupNouveau"
     * also, use vg.size() to avoid comparing the terminating \n */
    if(static_cast<unsigned long>(read_bytes) != vg.size() + 3 ||
       strncmp(buf + 2, vg.c_str(), vg.size())) {
        if(msgs) output_error(pos, "lvm_vg: volume group already exists and "
                              "is not using the specified physical volume");
    } else {
        /* the VG already exists and uses the specified PV - we're good */
        success = true;
    }

    free(buf);
    return success;
}
#endif /* HAS_INSTALL_ENV */

bool LVMGroup::execute() const {
    output_info(pos, "lvm_vg: creating volume group " + _vgname + " on " + _pv);

    if(script->options().test(Simulate)) {
        std::cout << "vgcreate " << _vgname << " " << _pv << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    /* REQ: Runner.Execute.lvm_vg.Duplicate */
    if(fs::exists("/dev/" + _vgname)) {
        return does_vg_exist_on_pv(_vgname, _pv, pos, true);
    }

    if(run_command("vgcreate", {_vgname, _pv}) != 0) {
        if(does_vg_exist_on_pv(_vgname, _pv, pos, true)) {
            return true;
        }

        output_error(pos, "lvm_vg: failed to create volume group " + _vgname);
        return false;
    }
#endif /* HAS_INSTALL_ENV */
    return true;  /* LCOV_EXCL_LINE */
}


Key *LVMVolume::parseFromData(const std::string &data,
                              const ScriptLocation &pos, int *errors, int *,
                              const Script *script) {
    std::string vg, name, size_str;
    std::string::size_type name_start, size_start;
    SizeType size_type;
    uint64_t size;

    long spaces = std::count(data.cbegin(), data.cend(), ' ');
    if(spaces != 2) {
        if(errors) *errors += 1;
        output_error(pos, "lvm_lv: expected 3 elements, got: " +
                     std::to_string(spaces),
                     "syntax is: lvm_lv [vg] [name] [size]");
        return nullptr;
    }

    name_start = data.find_first_of(' ');
    vg = data.substr(0, name_start);
    size_start = data.find_first_of(' ', name_start + 1);
    name = data.substr(name_start + 1, size_start - name_start - 1);
    size_str = data.substr(size_start + 1);

    if(!is_valid_lvm_name(vg)) {
        if(errors) *errors += 1;
        output_error(pos, "lvm_lv: invalid volume group name");
        return nullptr;
    }

    if(!is_valid_lvm_lv_name(name)) {
        if(errors) *errors += 1;
        output_error(pos, "lvm_lv: invalid volume name");
        return nullptr;
    }

    if(!parse_size_string(size_str, &size, &size_type)) {
        if(errors) *errors += 1;
        output_error(pos, "lvm_lv: invalid size", size_str);
        return nullptr;
    }

    return new LVMVolume(script, pos, vg, name, size_type, size);
}

bool LVMVolume::validate() const {
    return true;
}

bool LVMVolume::execute() const {
    output_info(pos, "lvm_lv: creating volume " + _lvname + " on " + _vg);
    std::string param, size;

    switch(_size_type) {
    case Fill:
        param = "-l";
        size = "100%FREE";
        break;
    case Bytes:
        param = "-L";
        size = std::to_string(_size) + "B";
        break;
    case Percent:
        param = "-l";
        size = std::to_string(_size) + "%VG";
        break;
    }

    if(script->options().test(Simulate)) {
        std::cout << "lvcreate " << param << " " << size << " -n "
                  << _lvname << " " << _vg << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    if(run_command("lvcreate", {param, size, "-n", _lvname, _vg}) != 0) {
        output_error(pos, "lvm_lv: failed to create logical volume " + _lvname);
        return false;
    }
#endif /* HAS_INSTALL_ENV */
    return true;  /* LCOV_EXCL_LINE */
}
