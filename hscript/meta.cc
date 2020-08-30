/*
 * meta.cc - Implementation of the Key classes for system metadata
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <assert.h>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#ifdef HAS_INSTALL_ENV
#   include <sys/mount.h>
#   include "util/filesystem.hh"
#endif /* HAS_INSTALL_ENV */
#include <unistd.h>         /* access - used by tz code even in RT env */
#include "meta.hh"
#include "util.hh"
#include "util/output.hh"

using namespace Horizon::Keys;

Key *Hostname::parseFromData(const std::string &data, const ScriptLocation &pos,
                             int *errors, int *, const Script *script) {
    std::string valid_chars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.");
    if(data.find_first_not_of(valid_chars) != std::string::npos) {
        if(errors) *errors += 1;
        output_error(pos, "hostname: expected machine or DNS name",
                     "'" + data + "' is not a valid hostname");
        return nullptr;
    }
    return new Hostname(script, pos, data);
}

bool Hostname::validate() const {
    /* Validate that the name is a valid machine or DNS name */
    bool any_failure = false;
    std::string::size_type last_dot = 0, next_dot = 0;

    if(!isalnum(this->_value[0])) {
        any_failure = true;
        output_error(pos, "hostname: must start with alphanumeric character");
    }

    if(this->_value.size() > 320) {
        any_failure = true;
        output_error(pos, "hostname: value too long",
                     "valid host names must be less than 320 characters");
    }

    do {
        next_dot = this->_value.find_first_of('.', next_dot + 1);
        if(next_dot == std::string::npos) {
            next_dot = this->_value.size();
        }
        if(next_dot - last_dot > 64) {
            any_failure = true;
            output_error(pos, "hostname: component too long",
                         "each component must be less than 64 characters");
        }
        last_dot = next_dot;
    } while(next_dot != this->_value.size());

    return !any_failure;
}

bool Hostname::execute() const {
    /* Set the hostname of the target computer */
    std::string actual;
    std::string::size_type dot = this->_value.find_first_of('.');

    if(this->_value.size() > 64) {
        /* Linux has a nodename limit of 64 characters.
         * That's fine, because we have a limit of 64 chars per segment.
         * Assuming a dot is present, just chop at the first dot. */
        assert(dot <= 64);
        actual = this->_value.substr(0, dot);
    } else {
        actual = this->_value;
    }

    /* Runner.Execute.hostname. */
    output_info(pos, "hostname: set hostname to '" + actual + "'");
    if(script->options().test(Simulate)) {
        std::cout << "hostname " << actual << std::endl;
    }
#ifdef HAS_INSTALL_ENV
    else if(script->options().test(ImageOnly)) {
        /* no-op; we don't want to set the image builder's hostname */
    } else {
        if(sethostname(actual.c_str(), actual.size()) == -1) {
            output_error(pos, "hostname: failed to set host name",
                         ::strerror(errno));
            return false;
        }
    }
#endif /* HAS_INSTALL_ENV */

    /* Runner.Execute.hostname.Write. */
    output_info(pos, "hostname: write '" + actual + "' to /etc/hostname");
    if(script->options().test(Simulate)) {
        std::cout << "mkdir -p " << script->targetDirectory() << "/etc"
                  << std::endl;
        std::cout << "printf '%s' " << actual << " > "
                  << script->targetDirectory() << "/etc/hostname" << std::endl;
    }
#ifdef HAS_INSTALL_ENV
    else {
        error_code ec;
        fs::create_directory(script->targetDirectory() + "/etc", ec);
        if(ec && ec.value() != EEXIST) {
            output_error(pos, "hostname: could not create /etc", ec.message());
            return false;
        }
        std::ofstream hostname_f(script->targetDirectory() + "/etc/hostname",
                                 std::ios_base::trunc);
        if(!hostname_f) {
            output_error(pos, "hostname: could not open /etc/hostname");
            return false;
        }
        hostname_f << actual;
    }
#endif /* HAS_INSTALL_ENV */

    /* The second condition ensures that it isn't a single dot that simply
     * terminates the nodename. */
    if(dot != std::string::npos && this->_value.length() > dot + 1) {
        const std::string domain(this->_value.substr(dot + 1));
        output_info(pos, "hostname: set domain name '" + domain + "'");
        if(script->options().test(Simulate)) {
            std::cout << "mkdir -p " << script->targetDirectory()
                      << "/etc/conf.d" << std::endl;
            std::cout << "printf 'dns_domain_lo=\"" << domain
                      << "\"\\" << "n' >> " << script->targetDirectory()
                      << "/etc/conf.d/net" << std::endl;
        }
#ifdef HAS_INSTALL_ENV
        else {
            if(!fs::exists(script->targetDirectory() + "/etc/conf.d")) {
                error_code ec;
                fs::create_directory(script->targetDirectory() +
                                     "/etc/conf.d", ec);
                if(ec) {
                    output_error(pos, "hostname: could not create /etc/conf.d "
                                 "directory", ec.message());
                }
            }
            std::ofstream net_conf_f(script->targetDirectory() +
                                     "/etc/conf.d/net", std::ios_base::app);
            if(!net_conf_f) {
                output_error(pos, "hostname: could not open /etc/conf.d/net "
                             "for writing");
                return false;
            }
            net_conf_f << "dns_domain_lo=\"" << domain << "\"" << std::endl;
        }
#endif /* HAS_INSTALL_ENV */
    }

    return true;
}


static std::set<std::string> valid_arches = {
    "aarch64", "aarch64_be", "alpha", "armel", "armhf", "armv7",
    "m68k", "mips", "mips64", "mipsel", "mips64el",
    "pmmx", "ppc", "ppc64",
    "riscv", "riscv64",
    "s390x", "sparc", "sparc64",
    "x86", "x86_64"
};


Key *Arch::parseFromData(const std::string &data, const ScriptLocation &pos,
                         int *errors, int *warnings, const Script *script) {
    if(data.find_first_not_of("abcdefghijklmnopqrstuvwyxz1234567890_") !=
            std::string::npos) {
        if(errors) *errors += 1;
        output_error(pos, "arch: expected CPU architecture name",
                     "'" + data + "' is not a valid CPU architecture name");
        return nullptr;
    }

    if(valid_arches.find(data) == valid_arches.end()) {
        if(warnings) *warnings += 1;
        output_warning(pos, "arch: unknown CPU architecture '" + data + "'");
    }

    return new Arch(script, pos, data);
}

bool Arch::execute() const {
    output_info(pos, "arch: setting system CPU architecture to " + value());

    if(script->options().test(Simulate)) {
        std::cout << "printf '" << this->value() << "\\" << "n'"
                  << " > " << script->targetDirectory() << "/etc/apk/arch"
                  << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    std::ofstream arch_f(script->targetDirectory() + "/etc/apk/arch",
                         std::ios_base::trunc);
    if(!arch_f) {
        output_error(pos, "arch: could not write target CPU architecture");
        return false;
    }

    arch_f << this->value() << std::endl;
#endif  /* HAS_INSTALL_ENV */
    return true;  /* LCOV_EXCL_LINE */
}


static std::regex valid_pkg("[0-9A-Za-z+_.-]*((>?<|[<>]?=|[~>])[0-9A-Za-z-_.]+)?");


Key *PkgInstall::parseFromData(const std::string &data,
                               const ScriptLocation &pos, int *errors,
                               int *warnings, const Script *script) {
    std::string next_pkg;
    std::istringstream stream(data);
    std::set<std::string> all_pkgs;

    while(stream >> next_pkg) {
        if(!std::regex_match(next_pkg, valid_pkg)) {
            if(errors) *errors += 1;
            output_error(pos, "pkginstall: expected package name",
                         "'" + next_pkg + "' is not a valid package or atom");
            return nullptr;
        }
        if(all_pkgs.find(next_pkg) != all_pkgs.end()) {
            if(warnings) *warnings += 1;
            output_warning(pos, "pkginstall: package '" + next_pkg +
                           "' is already in the target package set");
            continue;
        }
        all_pkgs.insert(next_pkg);
    }
    return new PkgInstall(script, pos, all_pkgs);
}


/* LCOV_EXCL_START */
bool PkgInstall::validate() const {
    /* Any validation errors would have occurred above. */
    return true;
}


bool PkgInstall::execute() const {
    /* Package installation is handled in Script::execute. */
    return true;
}
/* LCOV_EXCL_STOP */


/* All ISO 639-1 language codes.
 * Source: https://www.loc.gov/standards/iso639-2/ISO-639-2_utf-8.txt
 * Python to construct table:
 * >>> f = open('ISO-639-2_utf-8.txt')
 * >>> x = csv.reader(f, delimiter='|')
 * >>> langs = [lang[2] for lang in iter(x) if lang != '']
 * >>> print('"' + '", "'.join(langs) + '", "C."')
 */
const std::set<std::string> valid_langs = {
    "aa", "ab", "af", "ak", "sq", "am", "ar", "an", "hy", "as", "av", "ae",
    "ay", "az", "ba", "bm", "eu", "be", "bn", "bh", "bi", "bs", "br", "bg",
    "my", "ca", "ch", "ce", "zh", "cu", "cv", "kw", "co", "cr", "cs", "da",
    "dv", "nl", "dz", "en", "eo", "et", "ee", "fo", "fj", "fi", "fr", "fy",
    "ff", "ka", "de", "gd", "ga", "gl", "gv", "el", "gn", "gu", "ht", "ha",
    "he", "hz", "hi", "ho", "hr", "hu", "ig", "is", "io", "ii", "iu", "ie",
    "ia", "id", "ik", "it", "jv", "ja", "kl", "kn", "ks", "kr", "kk", "km",
    "ki", "rw", "ky", "kv", "kg", "ko", "kj", "ku", "lo", "la", "lv", "li",
    "ln", "lt", "lb", "lu", "lg", "mk", "mh", "ml", "mi", "mr", "ms", "mg",
    "mt", "mn", "na", "nv", "nr", "nd", "ng", "ne", "nn", "nb", "no", "ny",
    "oc", "oj", "or", "om", "os", "pa", "fa", "pi", "pl", "pt", "ps", "qu",
    "rm", "ro", "rn", "ru", "sg", "sa", "si", "sk", "sl", "se", "sm", "sn",
    "sd", "so", "st", "es", "sc", "sr", "ss", "su", "sw", "sv", "ty", "ta",
    "tt", "te", "tg", "tl", "th", "bo", "ti", "to", "tn", "ts", "tk", "tr",
    "tw", "ug", "uk", "ur", "uz", "ve", "vi", "vo", "cy", "wa", "wo", "xh",
    "yi", "yo", "za", "zu", "C."
};


Key *Language::parseFromData(const std::string &data, const ScriptLocation &pos,
                             int *errors, int *, const Script *script) {
    if(data.length() < 2 ||
       valid_langs.find(data.substr(0, 2)) == valid_langs.end()) {
        if(errors) *errors += 1;
        output_error(pos, "language: invalid language specified",
                     "language must be a valid ISO 639-1 language code");
        return nullptr;
    }

    /* We know a valid language appears, but is it real? */
    if(data.length() > 2) {
        /* data[1] is . if language is C.UTF-8 */
        if(data[2] != '_' && data[1] != '.') {
            if(errors) *errors += 1;
            output_error(pos, "language: invalid language specified",
                         "language must be a valid ISO 639-1 language code, "
                         "optionally followed by '_' and a country code");
            return nullptr;
        }
        /* we don't really care about the country code, but we do care about
         * codeset - we (via musl) *only* support UTF-8. */
        std::string::size_type dot = data.find_first_of('.');
        if(dot != std::string::npos && data.substr(dot+1, 5) != "UTF-8") {
            if(errors) *errors += 1;
            output_error(pos, "language: invalid language specified",
                         "you cannot specify a non-UTF-8 codeset");
            return nullptr;
        }
    }

    return new Language(script, pos, data);
}

bool Language::execute() const {
    output_info(pos, "language: setting default system language to " + _value);

    if(script->options().test(Simulate)) {
        std::cout << "printf '#!/bin/sh\\" << "nexport LANG=\"%s\"\\" << "n' "
                  << this->value() << " > " << script->targetDirectory()
                  << "/etc/profile.d/00-language.sh" << std::endl
                  << "chmod a+x " << script->targetDirectory()
                  << "/etc/profile.d/00-language.sh" << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    std::string lang_path = script->targetDirectory() +
            "/etc/profile.d/00-language.sh";
    std::ofstream lang_f(lang_path, std::ios_base::trunc);
    error_code ec;
    if(!lang_f) {
        output_error(pos, "language: could not open profile for writing");
        return false;
    }
    lang_f << "#!/bin/sh" << std::endl << "export LANG=\""
           << this->value() << "\"" << std::endl;
    lang_f.close();

    fs::permissions(lang_path, rwxr_xr_x, ec);
    if(ec) {
        output_error(pos, "language: could not set profile script "
                     "executable", ec.message());
        return false;
    }
#endif /* HAS_INSTALL_ENV */
    return true;  /* LCOV_EXCL_LINE */
}


#include "util/keymaps.hh"

Key *Keymap::parseFromData(const std::string &data, const ScriptLocation &pos,
                           int *errors, int *, const Script *script) {
    if(valid_keymaps.find(data) == valid_keymaps.end()) {
        if(errors) *errors += 1;
        output_error(pos, "keymap: invalid keymap specified");
        return nullptr;
    }

    return new Keymap(script, pos, data);
}

bool Keymap::validate() const {
    return true;
}

bool Keymap::execute() const {
    const std::string conf("keymap=\"" + _value + "\"\n\
windowkeys=\"NO\"\n\
extended_keymaps=\"\"\n\
dumpkeys_charset=\"\"\n\
fix_euro=\"NO\""
                           );

    output_info(pos, "keymap: setting system keyboard map to " + _value);

    if(script->options().test(Simulate)) {
        std::cout << "cat >" << script->targetDirectory()
                  << "/etc/conf.d/keymaps <<-KEYCONF" << std::endl;
        std::cout << conf << std::endl;
        std::cout << "KEYCONF" << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    std::ofstream keyconf(script->targetDirectory() + "/etc/conf.d/keymaps",
                          std::ios_base::trunc);
    if(!keyconf) {
        output_error(pos, "keymap: cannot write target keyboard configuration");
        return false;
    }

    keyconf << conf;
#endif /* HAS_INSTALL_ENV */
    return true;  /* LCOV_EXCL_LINE */
}


Key *Firmware::parseFromData(const std::string &data, const ScriptLocation &pos,
                             int *errors, int *, const Script *script) {
    bool value;
    if(!BooleanKey::parse(data, pos, "firmware", &value)) {
        if(errors) *errors += 1;
        return nullptr;
    }

    if(value) {
#ifdef NON_LIBRE_FIRMWARE
        output_warning(pos, "firmware: You have requested non-libre firmware.  "
                       "This may cause security issues, system instability, "
                       "and many other issues.  You should not enable this "
                       "option unless your system absolutely requires it.");
#else
        if(errors) *errors += 1;
        output_error(pos, "firmware: You have requested non-libre firmware, "
                     "but this version of Horizon does not support "
                     "non-libre firmware.", "Installation cannot proceed.");
        return nullptr;
#endif
    }
    return new Firmware(script, pos, value);
}

/* LCOV_EXCL_START */
bool Firmware::execute() const {
    /* By itself, this does nothing. */
    return true;
}
/* LCOV_EXCL_STOP */


Key *Timezone::parseFromData(const std::string &data, const ScriptLocation &pos,
                             int *errors, int *warnings, const Script *script) {
    if(data.find_first_of(" .\\") != std::string::npos || data[0] == '/') {
        if(errors) *errors += 1;
        output_error(pos, "timezone: invalid timezone name");
        return nullptr;
    }

    if(access("/usr/share/zoneinfo", X_OK) != 0) {
        if(warnings) *warnings += 1;
        output_warning(pos, "timezone: can't determine validity of timezone",
                       "zoneinfo data is missing or inaccessible");
    } else {
        std::string zi_path = "/usr/share/zoneinfo/" + data;
        if(access(zi_path.c_str(), F_OK) != 0) {
            if(errors) *errors += 1;
            output_error(pos, "timezone: unknown timezone '" + data + "'");
            return nullptr;
        }
    }

    return new Timezone(script, pos, data);
}

bool Timezone::execute() const {
    output_info(pos, "timezone: setting system timezone to " + this->value());

    if(script->options().test(Simulate)) {
        /* If the target doesn't have tzdata installed, copy the zoneinfo from
         * the Horizon environment. */
        std::cout << "([ -f " << script->targetDirectory()
                  << "/usr/share/zoneinfo/" << this->value()
                  << " ] && ln -s /usr/share/zoneinfo/" << this->value()
                  << " " << script->targetDirectory() << "/etc/localtime) || "
                  << "cp /usr/share/zoneinfo/" << this->value()
                  << " " << script->targetDirectory() << "/etc/localtime"
                  << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    std::string zi_path = "/usr/share/zoneinfo/" + this->value();
    std::string target_zi = script->targetDirectory() + zi_path;
    std::string target_lt = script->targetDirectory() + "/etc/localtime";
    error_code ec;
    if(fs::exists(target_lt, ec)) fs::remove(target_lt, ec);

    if(fs::exists(target_zi, ec)) {
        fs::create_symlink(zi_path, target_lt, ec);
        if(ec) {
            output_error(pos, "timezone: failed to create symbolic link",
                         ec.message());
            return false;
        }
        return true;
    } else {
        /* The target doesn't have tzdata installed.  We copy the zoneinfo
         * file from the Horizon environment to the target. */
        fs::copy_file(zi_path, target_lt, ec);
        if(ec) {
            output_error(pos, "timezone: failed to prepare target environment",
                         ec.message());
            return false;
        }
        return true;
    }
#else
    return false;  /* LCOV_EXCL_LINE */
#endif
}


Key *Repository::parseFromData(const std::string &data,
                               const ScriptLocation &pos, int *errors, int *,
                               const Script *script) {
    if(data.empty() || (data[0] != '/' && data.compare(0, 4, "http"))) {
        if(errors) *errors += 1;
        output_error(pos, "repository: must be absolute path or HTTP(S) URL");
        return nullptr;
    }
    return new Repository(script, pos, data);
}

bool Repository::validate() const {
    /* TODO XXX: Ensure URL is accessible if networking is available */
    return true;
}

bool Repository::execute() const {
    /* Runner.Execute.repository. */
    output_info(pos, "repository: write '" + this->value() +
                "' to /etc/apk/repositories");
    if(script->options().test(Simulate)) {
        std::cout << "echo '" << this->value()
                  << "' >> " << script->targetDirectory()
                  << "/etc/apk/repositories" << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    std::ofstream repo_f(script->targetDirectory() + "/etc/apk/repositories",
                         std::ios_base::app);
    if(!repo_f) {
        output_error(pos, "repository: could not open /etc/apk/repositories "
                     "for writing");
        return false;
    }

    repo_f << this->value() << std::endl;

    return true;
#else
    return false;  /* LCOV_EXCL_LINE */
#endif /* HAS_INSTALL_ENV */
}


Key *SigningKey::parseFromData(const std::string &data,
                               const ScriptLocation &pos,
                               int *errors, int *, const Script *script) {
    if(data.empty() || (data[0] != '/' && data.compare(0, 8, "https://"))) {
        if(errors) *errors += 1;
        output_error(pos, "signingkey: must be absolute path or HTTPS URL");
        return nullptr;
    }

    return new SigningKey(script, pos, data);
}

bool SigningKey::validate() const {
    return true;
}

bool SigningKey::execute() const {
    /* everything after the last / in the value is the filename */
    const std::string name(_value.substr(_value.find_last_of('/') + 1));

    const std::string target_dir(script->targetDirectory() + "/etc/apk/keys/");
    const std::string target(target_dir + name);

    output_info(pos, "signingkey: trusting " + name + " for APK signing");

    if(script->options().test(Simulate)) {
        std::cout << "mkdir -p " << target_dir << std::endl;
        if(_value[0] == '/') {
            std::cout << "cp " << _value << " " << target << std::endl;
        } else {
            std::cout << "curl -L -o " << target << " " << _value << std::endl;
        }
        return true;
    }

#ifdef HAS_INSTALL_ENV
    error_code ec;
    if(!fs::exists(target_dir)) {
        fs::create_directory(target_dir, ec);
        if(ec) {
            output_error(pos, "signingkey: could not initialise target "
                         "repository keys directory", ec.message());
            return false;
        }
    }

    if(_value[0] == '/') {
        fs::copy_file(_value, target, fs_overwrite, ec);
        if(ec) {
            output_error(pos, "signingkey: could not copy key to target",
                         ec.message());
            return false;
        }
    } else {
        return download_file(_value, target);
    }
#endif /* HAS_INSTALL_ENV */
    return true;  /* LCOV_EXCL_LINE */
}

Key *SvcEnable::parseFromData(const std::string &data,
                              const ScriptLocation &pos, int *errors, int *,
                              const Script *script) {
    const static std::string valid_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890.-_";
    std::string::size_type space = data.find_first_of(' ');;
    std::string svc, runlevel{"default"};

    if(space != std::string::npos) {
        svc = data.substr(0, space);
        runlevel = data.substr(space + 1);
    } else {
        svc = data;
    }

    if(svc.find_first_not_of(valid_chars) != std::string::npos) {
        if(errors) *errors += 1;
        output_error(pos, "svcenable: invalid service name", data);
        return nullptr;
    }

    return new SvcEnable(script, pos, svc, runlevel);
}

bool SvcEnable::validate() const {
    return true;  /* validation occurs during parsing */
}

bool SvcEnable::execute() const {
    const std::string target = script->targetDirectory() +
                               "/etc/runlevels/" + _runlevel + "/" + _svc;
    const std::string initd = "/etc/init.d/" + _svc;
    output_info(pos, "svcenable: enabling service " + _svc);

    if(script->options().test(Simulate)) {
        std::cout << "ln -s " << initd << " " << target << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    error_code ec;
    if(!fs::exists(script->targetDirectory() + initd, ec)) {
        output_warning(pos, "svcenable: missing service", _svc);
    }

    fs::create_symlink(initd, target, ec);
    if(ec && ec.value() != EEXIST) {
        output_error(pos, "svcenable: could not enable service " + _svc,
                     ec.message());
        return false;
    }
#endif  /* HAS_INSTALL_ENV */
    return true;  /* LCOV_EXCL_LINE */
}

Key *Version::parseFromData(const std::string &data,
                            const ScriptLocation &pos, int *errors, int *,
                            const Script *script) {
    const static std::string valid_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890.-_";

    if(data.find_first_not_of(valid_chars) != std::string::npos) {
        if(errors) *errors += 1;
        output_error(pos, "version: invalid version", data);
        return nullptr;
    }

    return new Version(script, pos, data);
}

bool Version::execute() const {
    return true;
}


const std::string my_arch(const Horizon::Script *script) {
    const Key *arch_key = script->getOneValue("arch");
    if(arch_key != nullptr) {
        const Arch *real_arch = dynamic_cast<const Arch *>(arch_key);
        return real_arch->value();
    } else {
#   if defined(__powerpc64__)
        return "ppc64";
#   elif defined(__powerpc__)
        return "ppc";
#   elif defined(__aarch64__)
        return "aarch64";
#   elif defined(__arm__)
        return "armv7";
#   elif defined(__i386__)
        return "pmmx";
#   elif defined(__x86_64__)
        return "x86_64";
#   elif defined(__mips64)
#       if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return "mips64el";
#       else  /* If byte order is not defined, default to big endian. */
        return "mips64";
#       endif
#   elif defined(__mips__)
#       if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return "mipsel";
#       else  /* If byte order is not defined, default to big endian. */
        return "mips";
#       endif
#   else
#       error Unknown architecture.
#   endif
    }
}

Key *Bootloader::parseFromData(const std::string &data,
                               const ScriptLocation &pos, int *errors, int *,
                               const Script *script) {
    const std::string arch = my_arch(script);
    std::string device, boot = "true";
    std::string::size_type space = data.find_first_of(" ");

    if(space == std::string::npos) {
        device = data;
    } else {
        device = data.substr(0, space);
        boot = data.substr(space + 1);

        if(boot.find_first_of(" ") != std::string::npos) {
            if(errors) *errors += 1;
            output_error(pos, "bootloader: invalid bootloader", data);
            return nullptr;
        }
    }

    if(boot == "true") {
        if(arch == "ppc64" || arch == "ppc") {
            boot = "grub-ieee1275";
        } else if(arch == "aarch64") {
            boot = "grub-efi";
        } else if(arch == "x86_64" || arch == "pmmx") {
#ifdef HAS_INSTALL_ENV
            if(fs::exists("/sys/firmware/efi")) {
                boot = "grub-efi";
            } else
#endif  /* HAS_INSTALL_ENV */
                boot = "grub-bios";
        } else {
            output_error(pos, "bootloader: no default for architecture", arch);
            return nullptr;
        }
    }

    return new Bootloader(script, pos, device, boot);
}

bool Bootloader::validate() const {
    const std::string arch = my_arch(script);
    bool valid_selection;
    const std::string candidate = this->bootloader();

    if(arch == "ppc64") {
        const static std::set<std::string> valid_ppc64 = {"grub-ieee1275"};
        valid_selection = valid_ppc64.find(candidate) != valid_ppc64.end();
    } else if(arch == "ppc") {
        const static std::set<std::string> valid_ppc = {"grub-ieee1275",
                                                        "iquik"};
        valid_selection = valid_ppc.find(candidate) != valid_ppc.end();
    } else if(arch == "aarch64") {
        const static std::set<std::string> valid_arm64 = {"grub-efi"};
        valid_selection = valid_arm64.find(candidate) != valid_arm64.end();
    } else if(arch == "armv7") {
        const static std::set<std::string> valid_arm = {};
        valid_selection = valid_arm.find(candidate) != valid_arm.end();
    } else if(arch == "pmmx") {
        const static std::set<std::string> valid_pmmx = {"grub-bios",
                                                         "grub-efi"};
        valid_selection = valid_pmmx.find(candidate) != valid_pmmx.end();
    } else if(arch == "x86_64") {
        const static std::set<std::string> valid_x86 = {"grub-bios",
                                                        "grub-efi"};
        valid_selection = valid_x86.find(candidate) != valid_x86.end();
    } else if(arch == "mips64" || arch == "mips" ||
              arch == "mips64el" || arch == "mipsel") {
        const static std::set<std::string> valid_mips = {};
        valid_selection = valid_mips.find(candidate) != valid_mips.end();
    } else {
        output_error(pos, "bootloader: unknown architecture", arch);
        return false;
    }

    if(!valid_selection) {
        output_error(pos, "bootloader: architecture does not support loader",
                     candidate);
    }

    return valid_selection;
}

bool Bootloader::execute() const {
    const std::string arch = my_arch(script);
    std::string method = _bootloader;

    if(method == "grub-efi") {
        if(script->options().test(Simulate)) {
            std::cout << "apk --root " << script->targetDirectory()
                      << " --keys-dir etc/apk/keys add grub-efi"
                      << std::endl
                      << "chroot " << script->targetDirectory()
                      << " grub-install " << _device << std::endl;
            return true;
        }
#ifdef HAS_INSTALL_ENV
        if(run_command("/sbin/apk",
                       {"--root", script->targetDirectory(), "--keys-dir",
                        "etc/apk/keys", "add", "grub-efi"}) != 0) {
            output_error(pos, "bootloader: couldn't add package");
            return false;
        }

        /* remount EFI vars r/w */
        const auto efipath{script->targetDirectory() +
                           "/sys/firmware/efi/efivars"};
        mount(nullptr, efipath.c_str(), nullptr,
              MS_REMOUNT | MS_NOEXEC | MS_NODEV | MS_NOSUID | MS_RELATIME,
              nullptr);

        if(run_command("chroot",
                       {script->targetDirectory(), "grub-install", _device})
                != 0) {
            output_error(pos, "bootloader: failed to install GRUB");
            return false;
        }

        /* done, back to r/o */
        mount(nullptr, efipath.c_str(), nullptr,
              MS_REMOUNT | MS_RDONLY | MS_NOEXEC | MS_NODEV | MS_NOSUID |
              MS_RELATIME, nullptr);
#endif  /* HAS_INSTALL_ENV */
        return true;  /* LCOV_EXCL_LINE */
    }
    else if(method == "grub-bios") {
        if(script->options().test(Simulate)) {
            std::cout << "apk --root " << script->targetDirectory()
                      << " --keys-dir etc/apk/keys add grub-bios"
                      << std::endl
                      << "chroot " << script->targetDirectory()
                      << " grub-install " << _device << std::endl;
            return true;
        }
#ifdef HAS_INSTALL_ENV
        if(run_command("/sbin/apk",
                       {"--root", script->targetDirectory(), "--keys-dir",
                        "etc/apk/keys", "add", "grub-bios"}) != 0) {
            output_error(pos, "bootloader: couldn't add package");
            return false;
        }
        if(run_command("chroot",
                       {script->targetDirectory(), "grub-install", _device})
                != 0) {
            output_error(pos, "bootloader: failed to install GRUB");
            return false;
        }
#endif  /* HAS_INSTALL_ENV */
        return true;  /* LCOV_EXCL_LINE */
    }
    else if(method == "iquik") {
        output_error(pos, "bootloader: iQUIK is not yet supported");
        return false;
    }
    else if(method == "grub-ieee1275") {
        if(script->options().test(Simulate)) {
            std::cout << "apk --root " << script->targetDirectory()
                      << " --keys-dir etc/apk/keys add grub-ieee1275"
                      << std::endl
                      << "chroot " << script->targetDirectory()
                      << " grub-install --macppc-directory=/boot/grub "
                      << _device << std::endl;
            return true;
        }
#ifdef HAS_INSTALL_ENV
        if(run_command("/sbin/apk",
                       {"--root", script->targetDirectory(), "--keys-dir",
                        "etc/apk/keys", "add", "grub-ieee1275"}) != 0) {
            output_error(pos, "bootloader: couldn't add package");
            return false;
        }
        if(run_command("chroot",
                       {script->targetDirectory(), "grub-install",
                        "--macppc-directory=/boot/grub", _device})
                != 0) {
            output_error(pos, "bootloader: failed to install GRUB");
            return false;
        }
#endif  /* HAS_INSTALL_ENV */
        return true;  /* LCOV_EXCL_LINE */
    }

    return false;  /* LCOV_EXCL_LINE */
}
