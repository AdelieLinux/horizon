============================
 README for Boot to Horizon
============================
:Status:
 Release Candidate
:Copyright:
 © 2015-2020 Adélie Linux.
 Code: AGPL-3.0 license.
 Documentation: CC BY-NC-SA open source license.


Boot to Horizon is the component of Project Horizon controlling the boot
environment for Horizon-based System Installation media.  This includes
configuration and scripts.



Introduction
============

This directory contains the development documentation, configuration,
back-end source code, and front-end source code for Boot to Horizon.


License
```````
Development documentation for Project Horizon is licensed under the
Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.

You should have received a copy of the license along with this
work. If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.

Code is licensed under the Affero GPL (AGPL) 3 license.


Changes
```````
Any changes to this repository must be reviewed before being pushed to the
master branch.



Directory Layout
================

``gfxdetect``
    OpenRC "service" to print a message about GPU detection.
    Installed as ``/etc/init.d/gfxdetect``.

``horizon-session.desktop``
    X session file.
    Installed as ``/usr/share/xsessions/horizon.desktop``.

``horizon.qss``
    Qt 5 stylesheet, making buttons slightly easier to use.
    Installed as ``/usr/share/qt5ct/qss/horizon.qss``.

``horizon.sddm.conf``
    SDDM configuration file to automatically start the Horizon wizard.
    Installed as ``/etc/sddm.conf.d/horizon.conf``.

``kwinrc``
    KWin configuration file.
    Installed as ``/etc/xdg/kwinrc``.

``qt5ct.conf``
    Qt 5 theme.
    Installed as ``/usr/share/qt5ct/qt5ct.conf``.

``start-horizon-qt5``
    Session controller script.
    Installed as ``/usr/bin/start-horizon-qt5``.



Reporting Issues
================

If you have an issue using Project Horizon, including with Boot to Horizon,
you may view our BTS_.  You may also `submit an issue`_ directly.

For general discussion, questions, or to submit a patch, please use the
`Horizon mailing list`_.

.. _BTS: https://bts.adelielinux.org/buglist.cgi?product=Horizon&resolution=---
.. _`submit an issue`: https://bts.adelielinux.org/enter_bug.cgi?product=Horizon
.. _`Horizon mailing list`: https://lists.adelielinux.org/postorius/lists/horizon.lists.adelielinux.org/

