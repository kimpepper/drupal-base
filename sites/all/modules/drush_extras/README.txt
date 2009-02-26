DESCRIPTION
------------
A collection of useful drush commands. See http://drupal.org/project/drush for more information.

USAGE
-----------
Install drush core from the above URL and invole these commands through it. The drush_extras module does not do anything on its own.

COMMANDS - SQL
-----------
The three most interesting commands are:

sql query: execute a query against the site database

sql dump: Exports the Drupal DB as SQL using mysqldump or pg_dump.

sql load: Migrate a database dump between two databases. Those databases are specified in your multi-site Drupal.


COMMANDS - Project Management (pm)
--------------
Allows you to install and update contributed modules from the command line.

It provides three commands, "pm install", "pm update", and "pm info".

Run "drush help pm install" and "drush help pm update" to see supported command line options and arguments.

If you use SVN for version control and want to suppress backup of modules when performing a `pm update`, enable drush_pm_svn.module.

drush_pm uses  wget (or curl), tar and gzip, so if you're trying to use drush_pm on Windows, you have to install these binaries beforehand. Try GnuWin32 (http://gnuwin32.sourceforge.net/).

COMMANDS - Tools
--------------------
Watchdog and rsync commands.


