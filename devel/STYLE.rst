======================================
 Code Style Guide for Project Horizon
======================================
:Author:
  * **A. Wilcox**, documentation writer
:Copyright:
  © 2019 Adélie Linux Team.  CC BY-NC-SA-4.0 open source licence.



Introduction
============

This style guide describes how the program code for Project Horizon is
written and styled.

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT",
"SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this
document are to be interpreted as described in `RFC 2119`_.

.. _`RFC 2119`: https://tools.ietf.org/html/rfc2119



Directory Layout
================

Tools that are applicable for use in the Runtime Environment and the
Installation Environment MUST be placed in the ``tools`` directory, in their
own subdirectory.

Tools that are used in the Installation Environment only MUST be placed in a
new top-level directory.

Implementation of HorizonScript keys MUST be written in a file in the
``hscript`` directory that describes the general category of their purpose.
For example, a metadata key belongs in ``meta.cc``.  If a key does not fit in
to any of the categories that currently exist, a new category MUST be created
for it.



Code Style
==========

Naming Conventions
------------------

C++ classes MUST be in the ``Horizon::`` namespace, and MUST be in CamelCase.

Variables MUST be all lowercase with underscores separating words.


Indentation
-----------

Tab stops MUST be 4 spaces.  Hard tabs MUST NOT be used.

Parenthetical statements that span multiple lines MUST have their continuation
aligned after the parenthetical.  For example:

::
    if(this->function() == "Fox" &&
       that->function() != "Vulpes")


Layout
------

Opening curly braces MUST be on the same line as the last line of the statement
that they open.  Closing curly braces MUST be on a separate line.

Closing curly braces MUST be indented at the same level as the statement they
are closing.  For example:

::
    if(foo) {
        bar();
    }


Commenting
----------

Comments MUST be written in complete, concise English sentences.

Blocks of code that are directly traceable to a requirement SHOULD have a REQ
comment, such as:

::
    /* REQ: Runner.Validate.foo */
    valid = foo->validate();
    ...

All methods MUST have a comment preceding their definition explaining their
purpose.  Methods SHOULD also use Doxygen-style notation for describing their
parameters, return values (if any), and side-effects (if any).


Encapsulation
-------------

C++ classes that are meant to be public API (that is, consumed by external
software) MUST use the PImpl_ pattern to encapsulate their members.

.. _PImpl: https://en.cppreference.com/w/cpp/language/pimpl
