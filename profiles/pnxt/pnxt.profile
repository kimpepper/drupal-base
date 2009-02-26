<?php
/**
* Return an array of the modules to be enabled when this profile is installed.
*
* @return
*  An array of modules to be enabled.
*/
function pnxt_profile_modules() {
  return array(
    // Enable required core modules first.
    'block', 'contact', 'filter', 'help', 'node', 'path', 'system', 'user', 'syslog',
   
    // Enable optional core modules next.
    'comment', 'help', 'menu', 'taxonomy', 'user',

    // Then, enable any contributed modules here.
    'addthis', 'admin_menu', 'advanced_help', 'backup_migrate', 'content', 
		'devel', 'drush', 'drush_mm', 'fckeditor', 
		'flag_content', 'globalredirect', 'googleanalytics', 'imageapi', 
		'imagecache', 'imagefield', 'imce', 'link','pathauto', 
		'print', 'simplead_block', 'token', 'views', 'views_ui', 'xmlsitemap'
	  );
}

/**
* Return a description of the profile for the initial installation screen.
*
* @return
*   An array with keys 'name' and 'description' describing this profile.
*/
function pnxt_profile_details() {
  return array(
    'name' => 'PreviousNext profile',
    'description' => 'This profile will install some commonly used contrib modules.',
  );
}

/**
* Implementation of hook_profile_final().
*
* pnxt platform installation.
*/
function pnxt_profile_final() {

}

?>