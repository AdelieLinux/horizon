========================================
 Contribution Guide for Project Horizon
========================================
:Author:
  * **A. Wilcox**, documentation writer
:Copyright:
  © 2015-2019 Adélie Linux Team.  NCSA open source licence.



Code Style
==========

See the `Code Style Guide`_ for information about the code style used in
Project Horizon.

.. _`Code Style Guide`: devel/STYLE.rst



Tests
=====

Please ensure that any feature additions include adequate testing.  The
overall goal for the Horizon repository is 100% test coverage for Runtime
Environment code, and 80% test coverage for Install Environment code.

Bug fixes may include regression tests to ensure the bug does not recur.



Contributing Changes
====================

This section describes the usual flows of contribution to this repository.
For a detailed description of how to contribute to Adélie Linux, review the
Handbook_.

.. _Handbook: https://help.adelielinux.org/html/devel/


GitLab Pull Requests
````````````````````

#. If you do not already have a GitLab account, you must create one.

#. Create a *fork* of the Horizon repository.  For more information, consult
   the GitLab online documentation.

#. Clone your forked repository to your computer.

#. Make your changes.

#. Test your changes to ensure they are correct.

#. Add (or remove) changed files using ``git add`` and ``git rm``.

#. Commit your changes to the tree using the commands ``git commit -S`` and
   ``git push``.

#. Visit your forked repository in a Web browser.

#. Choose the *Create Pull Request* button.

#. Review your changes to ensure they are correct, and then submit the form.


Mailing List
````````````

#. Clone the Horizon repository to your computer.

#. Make your changes.

#. Test your changes to ensure they are correct.

#. Add (or remove) changed files using ``git add`` and ``git rm``.

#. Commit your changes to the tree using the command ``git commit -S``.

#. Use the command ``git format-patch HEAD^`` to create a patch file for your
   commit.

   .. note:: If you have made multiple commits to the tree, you will need to
             add an additional ^ for each commit you have made.  For example,
             if you have made three commits, you will use the command
             ``git format-patch HEAD^^^``.

#. Email the resulting patch to the mailing list, using ``git send-email`` or
   your mail client.  Ensure you are subscribed_ first.

.. _subscribed: https://lists.adelielinux.org/postorius/lists/horizon.lists.adelielinux.org/
