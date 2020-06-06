============================
 README for Project Horizon
============================
:Authors:
 * **A. Wilcox**, principal author, project manager
 * **Laurent Bercot**, design and requirements
 * **Ariadne Conill**, design and requirements
 * **Elizabeth Myers**, former project champion
 * **Zach van Rijn**, design and requirements
 * **Alyx Wolcott**, analyst assistant
:Status:
 Development
:Copyright:
 © 2015-2020 Adélie Linux.
 Code: AGPL-3.0 license.
 Documentation: CC BY-NC-SA open source license.


.. image:: https://code.foxkit.us/adelie/horizon/raw/master/assets/horizon-256.png
   :target: https://horizon.adelielinux.org/
   :align: center
   :alt: Project Horizon


Project Horizon is the next-generation installation system for Adélie Linux.
It provides everyone with tools that make installation easy, inspectable,
auditable, secure, and fast.

.. image:: https://img.shields.io/badge/chat-on%20IRC-blue.svg
   :target: ircs://irc.interlinked.me:6697/#Adelie-Support
   :alt: Chat with us on IRC: irc.interlinked.me #Adelie-Support

.. image:: https://img.shields.io/badge/license-AGPLv3-lightgrey.svg
   :target: LICENSE-code
   :alt: Licensed under AGPLv3

.. image:: https://img.shields.io/gitlab/pipeline/adelie/horizon?gitlab_url=https%3A%2F%2Fcode.foxkit.us%2F
   :target: https://code.foxkit.us/adelie/horizon/pipelines
   :alt: Click for build status

.. image:: https://code.foxkit.us/adelie/horizon/badges/master/coverage.svg
   :target: https://horizon.adelielinux.org/coverage/
   :alt: Click for code coverage report

.. image:: https://img.shields.io/codacy/grade/fcca720981ee4646aa7e5b4f2f124aa4.svg
   :target: https://app.codacy.com/project/awilfox/horizon/dashboard
   :alt: Codacy code grade: A



Introduction
============

This repository contains the development documentation, script specification,
back-end source code, and front-end source code for Project Horizon.


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



If You Need Help
================

This repository is primarily for system developers.  If you're looking for
help using or installing Adélie Linux, see `our Help Centre on the Web`_.

.. _`our Help Centre on the Web`: https://help.adelielinux.org/



Build Requirements
==================

To build the entirety of Project Horizon, you will need:

* Qt 5 (Core; Network; Widgets) (qt5-qtbase-dev)

* cURL development files (curl-dev)

* Either a C++17 environment, or Boost.Filesystem (boost-dev)

  Horizon makes full use of the C++17 <filesystem> library if it is
  available, but will fall back to requiring Boost if it isn't.

To build the simulator and validator, you will additionally need:

* Boost.ProgramOptions (boost-dev)

To build Horizon UI for Installation Environments, you will additionally
need:

* GNU Parted development files (parted-dev)

* libudev development files (eudev-dev)

* `BCNM`_

* blkid and libfdisk development files (util-linux-dev)

* Boost.PropertyTree (for JSON parsing) (boost-dev)

* LibCap development files (libcap-dev)

* libX11 development files (libx11-dev)

* XKB File development files (libxkbfile-dev)

* Linux kernel header files (linux-headers)

To run the test suite, you will additionally need:

* RSpec (ruby-rspec)

* `Aruba`_

* Valgrind (valgrind)

If you wish to build Project Horizon statically (not common), you may add
'-static' to all target_link_libraries directives.  There is not yet a CMake
option to enable this behaviour, because it is not easy to test.

For full operation, Project Horizon requires the following packages to be
present on the computer where it runs (either the Runtime or Installation
Environment):

* tzdata (``timezone`` key validation and execution)

.. _`BCNM`: https://www.skarnet.org/software/bcnm/

.. _`Aruba`: https://github.com/cucumber/aruba


Build options
`````````````

The Horizon build system supports a few options to allow you to build the
components you need.

* ``BUILD_SHARED_LIBS``

  This may be ON or OFF.  If ON, libhscript will be a .so.  This results in
  significantly smaller build sizes, because the C++ runtime won't be built
  in to Horizon.  However, it may also sacrifice portability.

  The default is ON and is recommended you keep it as such.

* ``BUILD_TOOLS``

  This may be ON or OFF.  If ON, hscript-simulate and hscript-validate will
  be built.  It is recommended you leave it ON.

* ``COVERAGE``

  This may be ON or OFF.  If ON, code will be built with gcov support.
  Additionally, if LCOV (lcov) is installed on the system, you will be able
  to generate coverage reports.

  The default is OFF as this option is only useful for development of Horizon.

* ``VALGRIND``

  This option is only available if the Valgrind executable is found during
  CMake initialisation.  If ON, Valgrind tests will be run during the test
  suite.  The default is OFF.

* ``INSTALL``

  This option is only available if you are building Horizon on Linux, as it
  requires the Linux kernel headers (linux-headers).  If ON, the tools will
  support the Installation Environment and can be used to install Adélie
  Linux.  If OFF, the tools will only support the Runtime Environment,
  allowing you to validate and simulate HorizonScript files but not actually
  execute them.

  The default is ON when the build process is run on a computer running Linux.
  It is unavailable on other computers.



Repository Layout
=================

Project Horizon is laid out into multiple directories for ease of maintenance.

``assets``: Graphics and icons
``````````````````````````````
The ``assets`` directory contains UI and graphic files.


``boot``: Boot to Horizon resources
```````````````````````````````````
The ``boot`` directory contains resources for enabling Boot to Horizon, which
is used for the bootable Horizon-based System Installation media.


``build``: Build system artefacts
`````````````````````````````````
The ``build`` directory contains build output, including binaries and shared
libraries.


``devel``: Development information
``````````````````````````````````
The ``devel`` directory contains the Vision document, the Functional Software
Requirements Specification for Project Horizon, and the official HorizonScript
Specification.  The documents are written in DocBook XML; the HTML and PDF
versions are not stored in this repository.


``diskman``: Disk Manipulation library
``````````````````````````````````````
The ``diskman`` directory includes the source code for the DiskMan library,
which is used by Horizon front ends for probing and displaying block device
information.


``executor``: HorizonScript Executor
````````````````````````````````````
The ``executor`` directory includes the source code for the HorizonScript
Executor, which is the primary executable for running HorizonScript files.


``fetch``: HorizonScript Locator
````````````````````````````````
The ``fetch`` directory includes the source for the HorizonScript Locator,
used on the target computer to retrieve the installfile.


``hscript``: HorizonScript library
``````````````````````````````````
The ``hscript`` directory includes the source code for the HorizonScript
library.  This is the primary library for parsing, validating, and executing
HorizonScript files, and contains the principal code for Project Horizon.


``image``: Image creation tools
```````````````````````````````
The ``image`` directory includes the source code for generating images and
tarballs using HorizonScripts.  This is used for creating live media, pre-
generated installations for extraction to target computers, and more.  Note
that some tools may require a tool such as ``qemu-user`` to be installed on
the host, depending on installation options and desired target configuration.


``owner``: File ownership utility
`````````````````````````````````
The ``owner`` directory includes the source code for the ``hscript-printowner``
utility, which prints the UID of the owner of a given file.  This is used
during HorizonScript shell script execution.


``tests``: Test infrastructure
``````````````````````````````
The ``tests`` directory includes the ``fixtures`` directory, which is a
collection of dozens of example installfiles that exercise the parsing
and validation code of libhscript.  Some of these installfiles are
purposefully invalid, and others contain edge cases to ensure that the
library is written and implemented correctly.

It also contains the ``spec`` directory, which is a series of RSpec tests
designed to use the fixtures and ensure the correct output is given.


``tools``: Tooling and accessories
``````````````````````````````````
The ``tools`` directory includes the source code for tools related to Project
Horizon, including:

* The Validation Utility, which allows you to validate manually written
  installfiles.

* The Simulator, which allows you to view how the Horizon Runner would
  interpret your installfile.  The Simulator additionally allows you to
  output the interpretation of your installfile to a shell script.


``ui``: User interface code
```````````````````````````
The ``ui`` directory includes the source code for Project Horizon's front end,
including:

* ``qt5``, the Qt 5 user interface.


``util``: Shared utility code
`````````````````````````````
The ``util`` directory includes source code that is common between libraries
and tools.


``3rdparty``: External code
```````````````````````````
The ``3rdparty`` directory contains vendored code.  Currently, this is only
the qt-collapsible-section_ project, used by the Qt 5 UI for slide-out sections.

.. _qt-collapsible-section: https://github.com/Elypson/qt-collapsible-section



Contributing
============

See the CONTRIBUTING.rst_ file in the same directory as this README for
more details on how to contribute to Project Horizon.

.. _CONTRIBUTING.rst: ./CONTRIBUTING.rst



Reporting Issues
================

If you have an issue using Project Horizon, you may view our BTS_.  You may
also `submit an issue`_ directly.

For general discussion, questions, or to submit a patch, please use the
`Horizon mailing list`_.

.. _BTS: https://bts.adelielinux.org/buglist.cgi?product=Horizon&resolution=---
.. _`submit an issue`: https://bts.adelielinux.org/enter_bug.cgi?product=Horizon
.. _`Horizon mailing list`: https://lists.adelielinux.org/postorius/lists/horizon.lists.adelielinux.org/

