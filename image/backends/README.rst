======================================================
 README for Project Horizon - Image Creation Backends
======================================================
:Authors:
 * **A. Wilcox**, principal author, project manager
:Status:
 Development
:Copyright:
 © 2020 Adélie Linux.
 Code: AGPL-3.0 license.
 Documentation: CC BY-NC-SA open source license.


Introduction
============

This directory contains the various backends for creating images using
Project Horizon.


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



Design
======

Image creation backends shall derive from the Horizon::Image::BasicBackend
class.  Concrete backends can exist in any namespace.  To be used with the
Image Creation utility, the backend must be registered using the static
``BackendManager::register_backend`` function, using a unique Type Code.
This Type Code can then be passed to the Image Creation utility with the
``-t`` parameter for use.

The Horizon::Image::BasicBackend is an abstract (pure virtual) class that
has a single method that you must implement: ``create()``.  This will be
called on a constructed object of your concrete backend class, which has two
instance variables: ``std::string ir_dir``, which is the base directory for
the installed system (like ``/target`` during a normal installation), and
``std::string out_path``, which is the user's desired output path and file
name.  You may also implement ``prepare()``, which is called before create,
and ``finalise()``, which is called after.  Default no-op implementations
are provided for you in BasicBackend.



Repository Layout
=================

Each backend has its own .cc/.hh file.  The special ``backends.hh`` file
contains the table used by the Image Creation utility for determining
supported types and backends.



Contributing
============

See the CONTIRIBUTING.rst_ file in the roort directory of the Project Horizon
repository more details on how to contribute your own backend.

.. _CONTRIBUTING.rst: https://code.foxkit.us/adelie/horizon/blob/master/CONTRIBUTING.rst



Reporting Issues
================

If you have an issue using Project Horizon, you may view our BTS_.  You may
also `submit an issue`_ directly.

For general discussion, questions, or to submit a patch, please use the
`Horizon mailing list`_.

.. _BTS: https://bts.adelielinux.org/buglist.cgi?product=Horizon&resolution=---
.. _`submit an issue`: https://bts.adelielinux.org/enter_bug.cgi?product=Horizon
.. _`Horizon mailing list`: https://lists.adelielinux.org/postorius/lists/horizon.lists.adelielinux.org/

