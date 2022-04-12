Papo - Minimal IRC Server implementation
========================================

This is a work in progress, relatively untested and might be abandoned in the
near future. Use at your own risk.

DEVELOPMENT
-----------

Requirements

 1. gcc
 2. make

### Compiling

    $ make

### Testing

*Unit testing*

Make sure you have unit submodule cloned before run tests. You can run `git
submodule update --init` to get the submodules cloned.

    $ make test

*Functional testing*

Python3 is required to be able to run.

    $ make ftest

CONTRIBUTE
----------

We accept patches on our mailing list[^1] (use git-send-email to avoid issues
with your patches). We recommend you to follow the mailing etiquette[^2] before
send/reply emails to our mailing list.

[^1]: ~johnnyrichard/papo-devel@lists.sr.ht
[^2]: https://man.sr.ht/lists.sr.ht/etiquette.md
