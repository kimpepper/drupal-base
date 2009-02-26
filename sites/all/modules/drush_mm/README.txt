// $Id: README.txt,v 1.2 2008/08/07 11:42:12 clemenstolboom Exp $

DESCRIPTION
-----------

drush_mm.module (Drush Module Manager) allows you to enable and disable modules from the command line.

COMMANDS
--------
- list    : list the modules available on the given site
- enable  : enabled the given module and all modules it depends on
- disable : disable the given module and all modules depending on
- dot     : generated a dot file for further processing into a webpage

Run these as "drush <site> mm <command>"

Run "drush <site> help <command>" to see supported command line options and arguments.

REQUIREMENTS
------------
drush

------------
Written by Clemens Tolboom (ngnp) <http://build2be.com>.

No warranties of any kind. Use with care.