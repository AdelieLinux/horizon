===============================
 Changelog for Project Horizon
===============================
:Author:
  * **A. Wilcox**, documentation writer
  * **Contributors**, code
:Copyright:
  © 2019-2020 Adélie Linux and contributors.


0.9.3 (2020-07-06)
==================

Boot to Horizon
---------------

* Set X11 root window colour to calming blue.

* Disable compositing in KWin, which can cause issues in Qemu.


Build system
------------

* Only search for the BCNM library when building the Qt UI for Installation.

* Add ``BUILD_ISO`` toggle for building the ISO backend for Image Creation.


HorizonScript Library
---------------------

* Ensure mount directory tree exists before attempting to create the actual
  directory.

* Recursively bind-mount ``/dev`` and ``/sys`` in the target.

* When using the ImageOnly flag, don't ensure mountpoint devices are available.

* Fix issue where installation on live media failed due to network state.


Image Creation
--------------

* Add 'keep' option for ISO backend to preserve target dir between runs.

* Ensure ``init`` binary is executable in ISO initramfs.

* Correctly read scripts from stdin.


JSON
----

* Add ``service`` node support, which corresponds to the ``svcenable`` key.


Metadata
--------

* Add new ``bootloader`` key, to control installation of bootloaders.

* Fixes for EFI installations.

* Add optional runlevel parameter to ``svcenable`` key.


Network
-------

* Allow hostnames to start with numeric characters.  This represents a
  *potentially breaking change* to systems that require RFC 952 compliance.

* Add new ``pppoe`` key for configuring PPPoE links.


Qt UI
-----

* Properly flush log file to disk.

* Add workaround for network issue.

* Install SDDM for graphical installation types.

* Use bootloader key when a bootloader is requested.

* Fix various minor issues in custom package selection screen.

* Use ``vfat`` instead of ``fat32`` for ESP filesystem type.

* Correctly enable udev when udev is selected.




0.9.2 (2020-06-06)
==================

Boot to Horizon
---------------

* Added resources to enable booting to the System Installation Wizard.


Metadata
--------

* Update ``keymap`` to use kbd instead of console-setup.


Qt UI
-----

* Ensure log is opened properly when built in Release modes.

* Fix up signing keys, installed packages, and enabled services.




0.9.1 (2020-06-03)
==================

Build
-----

* Don't build the printowner utility in the Runtime Environment.


HorizonScript Library
---------------------

* Add new ScriptLocation object to more accurately report warnings and errors.

* Target mounts are no longer hardcoded to be a subdirectory of ``/target``.

* APK ``--keys-dir`` argument is now relative.

* Files downloaded from the network are written properly before execution
  continues, ensuring partial writes do not corrupt installation state.
 
* Execution of user keys works in cross-architecture script execution contexts.


Image Creation
--------------

* Run ``prepare`` before script execution.

* A root mount is no longer required to generate an image.

* The target is now a subdirectory of the IR dir, instead of the IR dir itself.

* Backends can now accept options via ``-b``.

* An ISO backend has been added, allowing generation of Live CDs.


Metadata
--------

* Add the ``svcenable`` key, allowing configuration of default services.

* Add the ``version`` key, allowing the version of Adélie to be selected.

* ``hostname``: Create ``/etc`` in the target if it does not yet exist.  This
  is required to properly generate Alpine Linux images.

* ``timezone``: Remove the existing ``/etc/localtime`` in all cases.


Qt UI
-----

* Fix building for Runtime Environment.

* Properly support High DPI screens.

* Properly check length of root passphrase.

* Allow mountpoints to be specified in the Runtime Environment.

* Generate a valid installfile if no packages are selected in Custom mode.

* Use QString helper to avoid GCC warning about snprintf bounds.




0.9.0 (2020-05-23)
==================

Build
-----

* The BCNM check is now functional.

Disk
----

* HFS+ has been added as a supported file system.

* disklabel: MBR support has been fixed.

* partition: Add 'bios' and 'prep' flags.

* partition: Type codes are now handled properly.

* partition: Ensure the first partition isn't too close to the first sectors.

* fs: Undo files for e2fs are no longer created.

* mount: Each mount key no longer overwrites /etc/fstab in the target.

DiskMan
-------

* A new library for probing disk information, DiskMan, has been written.

Documentation
-------------

* All tools and libraries, except libhscript, have been documented with
  manual pages.

HorizonScript Library
---------------------

* Introspection support has been added to the Script class.

* Refactored Keys to be owned by a Script, so Keys can access the values of
  other keys.

* The target directory is now configurable.

* /dev, /proc, and /sys are now mounted in the target.

Image Creation
--------------

* A new system for creating images using the Horizon system has been written.

Metadata
--------

* arch: New key added, including specification and implementation.

* repository: Fixed defaults when 'firmware' is set to true.

* timezone: Fixed issue when target already contained /etc/localtime.

Network
-------

* Configure network interfaces inside the Install Environment when
  'network' is set to true.

* netconfigtype: New key added, including specification and implementation.

* Existing network configuration on the Installation Environment system is
  now properly handled.

Package
-------

* APK Tools are now invoked more efficiently.

* --keys-dir is now passed to APK Tools during base installation.

Project
-------

* A Code of Conduct has been added.

* A YANG model, describing a JSON schema for representing a HorizonScript,
  has been written.

Tests
-----

* A test has been added for IPv4 subnet -> CIDR conversion.

Tools
-----

* Use Boost's program_options instead of vendoring clipp.  Thanks to
  Calvin Buckley.

* Messages have been unified between the CLI tools, to ensure consistency.

* A new tool, hscript-fromjson, has been added to convert JSON representations
  of HorizonScripts to HorizonScript.

UI
--

* The Qt 5 UI has been added.  It is not yet fully implemented, but basic
  installations should work.

User
----

* userpassphrase: Ensure simulated runs don't actually set passwords.

* All user keys now run shadow commands in the target, instead of the
  Installation Environment system.

Util
----

* Factor subnet -> CIDR conversion to util function.




0.2.0 (2019-11-07)
==================

Disk
----

* lvm_pv, lvm_vg, and lvm_lv execution are now implemented.


Metadata
--------

* keymap execution is now implemented.

* language: An issue with execution of the language key has been fixed.

* signingkey: Firmware keys are now installed when firmware is true.


Network
-------

* hostname: dns_domain_lo is now properly set in target /etc/conf.d/net.

* nameserver execution is now implemented.

* netaddress: OpenRC services are now added for configured interfaces.


Owner
-----

* New utility 'hscript-printowner' added, which prints the owning UID of a
  given path.


User
----

* User account creation is now fully implemented.




0.1.0 (2019-11-02)
==================

Initial release.
