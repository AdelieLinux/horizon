/*
 * meta.cc - Implementation of the Key classes for system metadata
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
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
#   include "util/filesystem.hh"
#endif /* HAS_INSTALL_ENV */
#include <unistd.h>         /* access - used by tz code even in RT env */
#include "meta.hh"
#include "util.hh"
#include "util/output.hh"

using namespace Horizon::Keys;

Key *Hostname::parseFromData(const std::string &data, int lineno, int *errors,
                             int *) {
    std::string valid_chars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.");
    if(data.find_first_not_of(valid_chars) != std::string::npos) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "hostname: expected machine or DNS name",
                     "'" + data + "' is not a valid hostname");
        return nullptr;
    }
    return new Hostname(lineno, data);
}

bool Hostname::validate(ScriptOptions) const {
    /* Validate that the name is a valid machine or DNS name */
    bool any_failure = false;
    std::string::size_type last_dot = 0, next_dot = 0;

    if(!isalpha(this->_value[0])) {
        any_failure = true;
        output_error("installfile:" + std::to_string(this->lineno()),
                     "hostname: must start with alphabetical character");
    }

    if(this->_value.size() > 320) {
        any_failure = true;
        output_error("installfile:" + std::to_string(this->lineno()),
                     "hostname: value too long",
                     "valid host names must be less than 320 characters");
    }

    do {
        next_dot = this->_value.find_first_of('.', next_dot + 1);
        if(next_dot == std::string::npos) {
            next_dot = this->_value.size();
        }
        if(next_dot - last_dot > 64) {
            any_failure = true;
            output_error("installfile:" + std::to_string(this->lineno()),
                         "hostname: component too long",
                         "each component must be less than 64 characters");
        }
        last_dot = next_dot;
    } while(next_dot != this->_value.size());

    return !any_failure;
}

bool Hostname::execute(ScriptOptions opts) const {
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
    output_info("installfile:" + std::to_string(this->lineno()),
                "hostname: set hostname to '" + actual + "'");
    if(opts.test(Simulate)) {
        std::cout << "hostname " << actual << std::endl;
    }
#ifdef HAS_INSTALL_ENV
    else {
        if(sethostname(actual.c_str(), actual.size()) == -1) {
            output_error("installfile:" + std::to_string(this->lineno()),
                         "hostname: failed to set host name",
                         std::string(strerror(errno)));
            return false;
        }
    }
#endif /* HAS_INSTALL_ENV */

    /* Runner.Execute.hostname.Write. */
    output_info("installfile:" + std::to_string(this->lineno()),
                "hostname: write '" + actual + "' to /etc/hostname");
    if(opts.test(Simulate)) {
        std::cout << "printf '%s' " << actual << " > /target/etc/hostname"
                  << std::endl;
    }
#ifdef HAS_INSTALL_ENV
    else {
        std::ofstream hostname_f("/target/etc/hostname", std::ios_base::trunc);
        if(!hostname_f) {
            output_error("installfile:" + std::to_string(this->lineno()),
                         "hostname: could not open /etc/hostname for writing");
            return false;
        }
        hostname_f << actual;
    }
#endif /* HAS_INSTALL_ENV */

    /* The second condition ensures that it isn't a single dot that simply
     * terminates the nodename. */
    if(dot != std::string::npos && this->_value.length() > dot + 1) {
        const std::string domain(this->_value.substr(dot + 1));
        output_info("installfile:" + std::to_string(this->lineno()),
                    "hostname: set domain name '" + domain + "'");
        if(opts.test(Simulate)) {
            std::cout << "mkdir -p /target/etc/conf.d" << std::endl;
            std::cout << "printf 'dns_domain_lo=\"" << domain
                      << "\"\\" << "n' >> /target/etc/conf.d/net" << std::endl;
        }
#ifdef HAS_INSTALL_ENV
        else {
            if(!fs::exists("/target/etc/conf.d")) {
                error_code ec;
                fs::create_directory("/target/etc/conf.d", ec);
                if(ec) {
                    output_error("installfile:" + std::to_string(line),
                                 "hostname: could not create /etc/conf.d "
                                 "directory", ec.message());
                }
            }
            std::ofstream net_conf_f("/target/etc/conf.d/net",
                                     std::ios_base::app);
            if(!net_conf_f) {
                output_error("installfile:" + std::to_string(this->lineno()),
                             "hostname: could not open /etc/conf.d/net for "
                             "writing");
                return false;
            }
            net_conf_f << "dns_domain_lo=\"" << domain << "\"" << std::endl;
        }
#endif /* HAS_INSTALL_ENV */
    }

    return true;
}


static std::regex valid_pkg("[0-9A-Za-z+_.-]*((>?<|[<>]?=|[~>])[0-9A-Za-z-_.]+)?");


Key *PkgInstall::parseFromData(const std::string &data, int lineno, int *errors,
                               int *warnings) {
    std::string next_pkg;
    std::istringstream stream(data);
    std::set<std::string> all_pkgs;

    while(stream >> next_pkg) {
        if(!std::regex_match(next_pkg, valid_pkg)) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "pkginstall: expected package name",
                         "'" + next_pkg + "' is not a valid package or atom");
            return nullptr;
        }
        if(all_pkgs.find(next_pkg) != all_pkgs.end()) {
            if(warnings) *warnings += 1;
            output_warning("installfile:" + std::to_string(lineno),
                           "pkginstall: package '" + next_pkg +
                           "' is already in the target package set");
            continue;
        }
        all_pkgs.insert(next_pkg);
    }
    return new PkgInstall(lineno, all_pkgs);
}


/* LCOV_EXCL_START */
bool PkgInstall::validate(ScriptOptions) const {
    /* Any validation errors would have occurred above. */
    return true;
}


bool PkgInstall::execute(ScriptOptions) const {
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


Key *Language::parseFromData(const std::string &data, int lineno, int *errors,
                             int *) {
    if(data.length() < 2 ||
       valid_langs.find(data.substr(0, 2)) == valid_langs.end()) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "language: invalid language specified",
                     "language must be a valid ISO 639-1 language code");
        return nullptr;
    }

    /* We know a valid language appears, but is it real? */
    if(data.length() > 2) {
        /* data[1] is . if language is C.UTF-8 */
        if(data[2] != '_' && data[1] != '.') {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "language: invalid language specified",
                         "language must be a valid ISO 639-1 language code, "
                         "optionally followed by '_' and a country code");
            return nullptr;
        }
        /* we don't really care about the country code, but we do care about
         * codeset - we (via musl) *only* support UTF-8. */
        std::string::size_type dot = data.find_first_of('.');
        if(dot != std::string::npos && data.substr(dot+1, 5) != "UTF-8") {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "language: invalid language specified",
                         "you cannot specify a non-UTF-8 codeset");
            return nullptr;
        }
    }

    return new Language(lineno, data);
}

bool Language::execute(ScriptOptions opts) const {
    output_info("installfile:" + std::to_string(this->lineno()),
                "language: setting default system language to " +
                this->value());

    if(opts.test(Simulate)) {
        std::cout << "printf '#!/bin/sh\\" << "nexport LANG=\"%s\"\\" << "n' "
                  << this->value() << " > /target/etc/profile.d/language.sh"
                  << std::endl
                  << "chmod a+x /target/etc/profile.d/language.sh"
                  << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    const char *lang_path = "/target/etc/profile.d/language.sh";
    std::ofstream lang_f(lang_path, std::ios_base::trunc);
    error_code ec;
    if(!lang_f) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "language: could not open /etc/profile.d/language.sh "
                     "for writing");
        return false;
    }
    lang_f << "#!/bin/sh" << std::endl << "export LANG=\""
           << this->value() << "\"" << std::endl;
    lang_f.close();

    fs::permissions(lang_path, rwxr_xr_x, ec);
    if(ec) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "language: could not set /etc/profile.d/language.sh "
                     "as executable", ec.message());
        return false;
    }
#endif /* HAS_INSTALL_ENV */
    return true;
}


const std::set<std::string> valid_keymaps = {
    "us", "ad", "af", "ara", "al", "am", "at", "az", "by", "be", "bd", "in",
    "ba", "br", "bg", "ma", "mm", "ca", "cd", "cn", "hr", "cz", "dk", "nl",
    "bt", "ee", "ir", "iq", "fo", "fi", "fr", "gh", "gn", "ge", "de", "gr",
    "hu", "is", "il", "it", "jp", "kg", "kh", "kz", "la", "latam", "lt", "lv",
    "mao", "me", "mk", "mt", "mn", "no", "pl", "pt", "ro", "ru", "rs", "si",
    "sk", "es", "se", "ch", "sy", "tj", "lk", "th", "tr", "ua", "gb", "uz",
    "vn", "kr", "ie", "pk", "mv", "za", "epo", "np", "ng", "et", "sn", "brai",
    "tm", "ml", "tz", "ke", "bw", "ph"
};

Key *Keymap::parseFromData(const std::string &data, int lineno, int *errors,
                           int *) {
    if(valid_keymaps.find(data) == valid_keymaps.end()) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "keymap: invalid keymap specified");
        return nullptr;
    }

    return new Keymap(lineno, data);
}

bool Keymap::validate(ScriptOptions) const {
    return true;
}

bool Keymap::execute(ScriptOptions opts) const {
    const std::string conf("# KEYBOARD CONFIGURATION FILE\n\
\n\
# Consult the keyboard(5) manual page.\n\
\n\
XKBMODEL=pc105\n\
XKBLAYOUT=" + _value + "\n\
XKBVARIANT=\n\
XKBOPTIONS=\n\
\n\
BACKSPACE=guess"
                           );

    output_info("installfile:" + std::to_string(line),
                "keymap: setting system keyboard map to " + _value);

    if(opts.test(Simulate)) {
        std::cout << "cat >/target/etc/default/keyboard <<-KEYCONF"
                  << std::endl;
        std::cout << conf << std::endl;
        std::cout << "KEYCONF" << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    std::ofstream keyconf("/target/etc/default/keyboard",
                          std::ios_base::trunc);
    if(!keyconf) {
        output_error("installfile:" + std::to_string(line),
                     "keymap: cannot write target keyboard configuration");
        return false;
    }

    keyconf << conf;
#endif /* HAS_INSTALL_ENV */
    return true;
}


Key *Firmware::parseFromData(const std::string &data, int lineno, int *errors,
                             int *) {
    bool value;
    if(!BooleanKey::parse(data, "installfile:" + std::to_string(lineno),
                          "firmware", &value)) {
        if(errors) *errors += 1;
        return nullptr;
    }

    if(value) {
#ifdef NON_LIBRE_FIRMWARE
        output_warning("installfile:" + std::to_string(lineno),
                       "firmware: You have requested non-libre firmware.  "
                       "This may cause security issues, system instability, "
                       "and many other issues.  You should not enable this "
                       "option unless your system absolutely requires it.");
#else
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "firmware: You have requested non-libre firmware, "
                     "but this version of Horizon does not support "
                     "non-libre firmware.", "Installation cannot proceed.");
        return nullptr;
#endif
    }
    return new Firmware(lineno, value);
}

bool Firmware::execute(ScriptOptions) const {
    /* By itself, this does nothing. */
    return true;
}


Key *Timezone::parseFromData(const std::string &data, int lineno, int *errors,
                             int *warnings) {
    if(data.find_first_of(" .\\") != std::string::npos || data[0] == '/') {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "timezone: invalid timezone name");
        return nullptr;
    }

    if(access("/usr/share/zoneinfo", X_OK) != 0) {
        if(warnings) *warnings += 1;
        output_warning("installfile:" + std::to_string(lineno),
                       "timezone: can't determine validity of timezone",
                       "zoneinfo data is missing or inaccessible");
    } else {
        std::string zi_path = "/usr/share/zoneinfo/" + data;
        if(access(zi_path.c_str(), F_OK) != 0) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "timezone: unknown timezone '" + data + "'");
            return nullptr;
        }
    }

    return new Timezone(lineno, data);
}

bool Timezone::execute(ScriptOptions opts) const {
    output_info("installfile:" + std::to_string(this->lineno()),
                "timezone: setting system timezone to " + this->value());

    if(opts.test(Simulate)) {
        /* If the target doesn't have tzdata installed, copy the zoneinfo from
         * the Horizon environment. */
        std::cout << "([ -f /target/usr/share/zoneinfo/" << this->value()
                  << " ] && ln -s /usr/share/zoneinfo/" << this->value()
                  << " /target/etc/localtime) || cp /usr/share/zoneinfo/"
                  << this->value() << " /target/etc/localtime" << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    std::string zi_path = "/usr/share/zoneinfo/" + this->value();
    std::string target_zi = "/target" + zi_path;
    error_code ec;
    if(fs::exists(target_zi, ec)) {
        fs::create_symlink(zi_path, "/target/etc/localtime", ec);
        if(ec) {
            output_error("installfile:" + std::to_string(this->lineno()),
                         "timezone: failed to create symbolic link",
                         ec.message());
            return false;
        }
        return true;
    } else {
        /* The target doesn't have tzdata installed.  We copy the zoneinfo
         * file from the Horizon environment to the target. */
        fs::copy_file(zi_path, "/target/etc/localtime", ec);
        if(ec) {
            output_error("installfile:" + std::to_string(this->lineno()),
                         "timezone: failed to prepare target environment",
                         ec.message());
            return false;
        }
        return true;
    }
#else
    return false;
#endif
}


Key *Repository::parseFromData(const std::string &data, int lineno, int *errors,
                               int *) {
    if(data.empty() || (data[0] != '/' && data.compare(0, 4, "http"))) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "repository: must be absolute path or HTTP(S) URL");
        return nullptr;
    }
    return new Repository(lineno, data);
}

bool Repository::validate(ScriptOptions) const {
    /* TODO XXX: Ensure URL is accessible if networking is available */
    return true;
}

bool Repository::execute(ScriptOptions opts) const {
    /* Runner.Execute.repository. */
    output_info("installfile:" + std::to_string(this->lineno()),
                "repository: write '" + this->value() +
                "' to /etc/apk/repositories");
    if(opts.test(Simulate)) {
        std::cout << "echo '" << this->value() <<
                     "' >> /target/etc/apk/repositories" << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    std::ofstream repo_f("/target/etc/apk/repositories",
                         std::ios_base::app);
    if(!repo_f) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "repository: could not open /etc/apk/repositories "
                     "for writing");
        return false;
    }

    repo_f << this->value() << std::endl;

    return true;
#else
    return false;
#endif /* HAS_INSTALL_ENV */
}


Key *SigningKey::parseFromData(const std::string &data, int lineno,
                               int *errors, int *) {
    if(data.empty() || (data[0] != '/' && data.compare(0, 8, "https://"))) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "signingkey: must be absolute path or HTTPS URL");
        return nullptr;
    }

    return new SigningKey(lineno, data);
}

bool SigningKey::validate(ScriptOptions) const {
    return true;
}

bool SigningKey::execute(ScriptOptions opts) const {
    /* everything after the last / in the value is the filename */
    const std::string name(_value.substr(_value.find_last_of('/') + 1));

    const std::string target("/target/etc/apk/keys/" + name);

    output_info("installfile:" + std::to_string(line),
                "signingkey: trusting " + name + " for repository signing");

    if(opts.test(Simulate)) {
        std::cout << "mkdir -p /target/etc/apk/keys" << std::endl;
        if(_value[0] == '/') {
            std::cout << "cp " << _value << " " << target << std::endl;
        } else {
            std::cout << "curl -L -o " << target << " " << _value << std::endl;
        }
        return true;
    }

#ifdef HAS_INSTALL_ENV
    error_code ec;
    if(!fs::exists("/target/etc/apk/keys")) {
        fs::create_directory("/target/etc/apk/keys", ec);
        if(ec) {
            output_error("installfile:" + std::to_string(line),
                         "signingkey: could not initialise target repository "
                         "keys directory", ec.message());
            return false;
        }
    }

    if(_value[0] == '/') {
        fs::copy_file(_value, target, fs_overwrite, ec);
        if(ec) {
            output_error("installfile:" + std::to_string(line),
                         "signingkey: could not copy signing key to target",
                         ec.message());
            return false;
        }
    } else {
        return download_file(_value, target);
    }
#endif /* HAS_INSTALL_ENV */
    return true;
}
