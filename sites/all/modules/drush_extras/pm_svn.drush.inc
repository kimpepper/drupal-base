<?php
// $Id: pm_svn.drush.inc,v 1.1 2008/12/07 05:12:11 weitzman Exp $

/**
 * @file Drush pm SVN extension
 */

/**
 * Implementation of hook_drush_help().
 */
function pm_svn_drush_help($section) {
  $help = '';
  switch ($section) {
    case 'drush:pm update':
    case 'drush:pm install':
      $help .= dt("--svnsync - automatically add new files to the Subversion repository and
remove deleted files. Use this with caution, especially if you leave files in
project directories that you don't want to be marked for addition, or you
have deleted a non-project file that you don't want to be marked in svn as a
deletion.\n
--svncommit - automatically commit project install or update changes to the
sites Subversion repository. You can only use this option if you are also
using the --svnsync option, because committing without first adding new and
removing deleted files will cause unpredictable results.\n
--svnmessage - Override the commit message from the default, which is:
'Drush automatic commit: ', and the drush command line used.\n
--svnstatusparams - add options to the 'svn status' command
--svnaddparams - add options to the 'svn add' command
--svnremoveparams - add options to the 'svn remove' command
--svncommitparams - add options to the 'svn commit' command
e.g. --svncommitparams=\"--username joe\" (note that quotes are required) will
commit changes as the user 'joe'.\n");
      return $help;
  }
}

/**
 * Implementation of hook_drush_pm_skip_backup().
 */
function pm_svn_drush_pm_skip_backup($source) {
  if (file_exists($source. '/.svn')) {
    // Skip backup since we need all .svn  directories. Assuming admin can use svn revert if new project is bad.
    drush_verbose(dt('Backup skipped because .svn directory was detected. Use Subversion to revert if needed.'));
    return TRUE;
  }
}

/**
 * Implementation of hook_drush_pm_post_update().
 */
function pm_svn_drush_pm_post_update($project, $info, $path = '.') {
  if (drush_pm_svn_sync($project, $info, $path)) {
    // Only attempt commit on a sucessful sync
    drush_pm_svn_commit($project, $info, $path);
  }
}

/**
 * Implementation of hook_drush_pm_post_install().
 */
function pm_svn_drush_pm_post_install($project, $info, $path = '.') {
  if (drush_pm_svn_sync($project, $info, $path)) {
    // Only attempt commit on a sucessful sync
    drush_pm_svn_commit($project, $info, $path);
  }
}

/**
 * Automatically add any unversioned files to Subversion and remove any files
 * that have been deleted on the file system
 */
function drush_pm_svn_sync($project, $info, $path = '.') {
  if (drush_get_option('svnsync')) {
    $errors = '';
    if (drush_shell_exec('svn status '. drush_get_option('svnstatusparams') .' '. $project)) {
      $output = drush_shell_exec_output();
      foreach ($output as $line) {
        if (preg_match('/^\? *(.*)/', $line, $matches)) {
          if (!drush_shell_exec('svn add '. drush_get_option('svnaddparams') .' '. $matches[1])) {
            $errors .= implode("\n", drush_shell_exec_output());
          }
        }
        if (preg_match('/^\! *(.*)/', $line, $matches)) {
          if (!drush_shell_exec('svn remove '. drush_get_option('svnremoveparams') .' '. $matches[1])) {
            $errors .= implode("\n", drush_shell_exec_output());
          }
        }
      }
      if (!empty($errors)) {
        drush_print(dt("Problems were encountered adding or removing files to/from subversion.\nThe specific errors are below:"));
        drush_print($errors);
        return FALSE;
      }
    }
    else {
      drush_print(dt("Drush was unable to get the svn status. Check that you have Subversion \ninstalled and that the site is a subversion working copy.\nThe specific errors are below:"));
      drush_print(implode("\n", drush_shell_exec_output()));
      return FALSE;
    }
    return TRUE;
  }
}

/**
 * Automatically commit changes to the repository
 */
function drush_pm_svn_commit($project, $info, $path = '.') {
  if (drush_get_option('svncommit')) {
    $message = drush_get_option('svnmessage');
    if (empty($message)) {
      $message = dt("Drush automatic commit: \n") . implode(' ', $_SERVER['argv']);
    }
    if (drush_shell_exec('svn commit '. drush_get_option('svncommitparams') .' -m "'. $message .'" '. $project)) {
      drush_print(dt('Project committed to Subversion successfully'));
    }
    else {
      drush_print(dt("'Problems were encountered committing your changes to Subversion.\nThe specific errors are below:"));
      drush_print(implode("\n", drush_shell_exec_output()));
    }
  }
  else {
    drush_print(dt("You should consider committing the new code to your Subversion repository.\nIf this version becomes undesireable, use Subversion to roll back."));
  }
}
