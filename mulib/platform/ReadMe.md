# The mulib/platform directory
This directory contains the files to adapt mulib to your specific environment.
It contains the following:
* `mu_config.h`: sets a few compile-time parameters to give you control over
mutlib.  See that file for more information.
* `mu_time.h`: Defines how your system defines time.
* `mu_time.c`: Implements times-specific functions, e.g. for getting the current
time, comparing two times, converting to and from milliseconds, etc.

## Pre-defined platform files
In the mulib repository, platforms/ contains a (growing) number of pre-made
platform directories, one of which might be suitable for your needs.  Before
you create your own platform-specific files from scratch, look here to see if
any of them are appropriate for your needs, or perhaps close to what you need.

## Defining your own platform files.
If none of the existing platform files match your environment, you will need to
create your own.  Copy the three boilerplate files from this directory into your
project and, using the comments in those files, edit them in order to adapt them
to your environment.
