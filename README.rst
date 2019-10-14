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
 © 2015-2019 Adélie Linux.
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

* Qt 5 (Core; Widgets) (qt5-qtbase-dev)

* GNU Parted development files (parted-dev)

* libudev development files (eudev-dev)

* BCNM

To run the test suite, you will additionally need:

* RSpec (ruby-rspec)

* Aruba

* Valgrind (valgrind)



Repository Layout
=================

Project Horizon is laid out into multiple directories for ease of maintenance.

``assets``: Graphics and icons
``````````````````````````````
The ``assets`` directory contains UI and graphic files.


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


``hscript``: HorizonScript library
``````````````````````````````````
The ``hscript`` directory includes the source code for the HorizonScript
library.  This is the primary library for parsing, validating, and executing
HorizonScript files, and contains the principal code for Project Horizon.


``tools``: Tooling and accessories
``````````````````````````````````
The ``tools`` directory includes the source code for tools related to Project
Horizon, including:

* The Validation Utility, which allows you to validate manually written
  installfiles.

* The Simulator, which allows you to view how the Horizon Runner would
  interpret your installfile.  The Simulator additionally allows you to
  output the interpretation of your installfile to a shell script.


``util``: Shared utility code
`````````````````````````````
The ``util`` directory includes source code that is common between libraries
and tools.


``tests``: Test infrastructure
``````````````````````````````
The ``tests`` directory includes the ``fixtures`` directory, which is a
collection of dozens of example installfiles that exercise the parsing
and validation code of libhscript.  Some of these installfiles are
purposefully invalid, and others contain edge cases to ensure that the
library is written and implemented correctly.

It also contains the ``spec`` directory, which is a series of RSpec tests
designed to use the fixtures and ensure the correct output is given.


``3rdparty``: External code
```````````````````````````
The ``3rdparty`` directory contains vendored code.  Currently, this is only
the clipp_ project, used by the ``tools`` for argument parsing.

.. _clipp: https://github.com/muellan/clipp



Contributing
============

See the CONTIRIBUTING.rst_ file in the same directory as this README for
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

