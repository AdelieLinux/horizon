===============================
 Changelog for Project Horizon
===============================
:Author:
  * **A. Wilcox**, documentation writer
  * **Contributors**, code
:Copyright:
  © 2019 Adélie Linux and contributors.


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
