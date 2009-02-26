<?php
// $Id: pm_cvs.drush.inc,v 1.1 2008/12/07 05:12:11 weitzman Exp $

/**
 * @file Drush PM CVS extension
 */

/**
 * Implementation of hook_drush_help().
 */
function pm_cvs_drush_help($section) {
  $help = '';
  switch ($section) {
    case 'drush:pm update':
      $help .= dt("--cvsmethod - force cvs updates or checkouts
e.g. --cvsmethod=update will update the module, and try to merge changes,
rather than overwriting them. Any conflicts will need to be resolved manually.
The checkout method is the default for everything except SVN managed projects.\n\n");
    case 'drush:pm install':
      // The --cvsparams option is available on both update and install
      $help .= dt("--cvsparams - add options to the cvs command
e.g. --cvsparams=\"-C\" (note that quotes are required) will overwrite all
changes when updating an SVN managed project.\n");
      return $help;
  }
}

/**
 * Hook drush_pm_module_handler
 */
function pm_cvs_drush_pm_package_handler() {
  return array('drush_pm_cvs');
}

/**
 * Install a project (so far, only modules are supported).
 *
 * @param $project The short name of the drupal.org project
 * @param $info The details (fetched from drupal.org via xml-rpc)
 * @param $path The path to install the module to.
 */
function drush_pm_cvs_install_project($project, $info, $path = '.') {
  drush_op('chdir', $path);

  drush_verbose("Downloading project $project ...");

  $cvsparams = drush_get_option('cvsparams');

  // Check it out.
  drush_pm_cvs_checkout($project, $info['tag'], $cvsparams);

  if (is_dir($path . $project) && !DRUSH_SIMULATE) {
    drush_verbose("Checking out " . $project . " was successful.");
    return TRUE;
  }
  else {
    return drush_error("Unable to check out $filename to $path from cvs.drupal.org");
  }
}

/**
 * Update a project (so far, only modules are supported).
 *
 * @param $project The short name of the drupal.org project
 * @param $info The details (fetched from drupal.org via xml-rpc)
 * @param $path The path to install the module to.
 */
function drush_pm_cvs_update_project($project, $info, $path = '.') {
  drush_op('chdir', $path);

  drush_verbose("Updating project $project ...");

  $cvsmethod = drush_get_option('cvsmethod');
  if (empty($cvsmethod) && file_exists($project. '/.svn')) {
    // If we have .svn files the default is to update in place
    // this will try and merge any changes, which could break things
    // but we assume anyone using SVN is competent enough to deal with this!
    $cvsmethod = 'update';
  }

  $cvsparams = drush_get_option('cvsparams');

  drush_verbose($cvsmethod);
  drush_verbose(strpos(strtolower($cvsmethod), 'up'));
  if (substr($cvsmethod, 0, 2) == 'up') {
    // Update the working copy.
    drush_op('chdir', $project);
    if (!drush_shell_exec('cvs update -dP '. $cvsparams . ' -r '. $info['tag'])) {
      drush_die("Unable to update $project from cvs.drupal.org.");
    }
    drush_op('chdir', '..');
  }
  else {
    // Check it out.
    drush_pm_cvs_checkout($project, $info['tag'], $cvsparams);
  }

  if (is_dir($path . $project) && !DRUSH_SIMULATE) {
    drush_verbose("Updating out " . $project . " was successful.");
    return TRUE;
  }
  else {
    return drush_error("Unable to update $project from cvs.drupal.org");
  }
}

function drush_pm_cvs_checkout($project, $tag, $cvsparams) {
  if (!drush_shell_exec('cvs -z6 '. $cvsparams .' -d:pserver:anonymous:anonymous@cvs.drupal.org:/cvs/drupal-contrib checkout -d '. $project .' -r '. $tag .' contributions/modules/'. $project)) {
    drush_die("Unable to checkout $project from cvs.drupal.org.");
  }
}