<?php
// $Id: tools.drush.inc,v 1.1 2008/12/07 05:12:11 weitzman Exp $

/**
 * @file
 *   Miscellaneous Drush tools
 */

/**
 * Implementation of hook_drush_help().
 */
function tools_drush_help($section) {
  switch ($section) {
    case 'drush:watchdog show':
      return dt("Usage: drush [options] watchdog show <n> <type>\n\nShow the <n> (default 10) most recent watchdog messages. Optionally show only messages of type <type>.");
    case 'drush:watchdog delete':
      return dt("Usage: drush [options] watchdog delete <type>\n\nDelete all messages of type <type>. Use 'all' to delete all messages, no matter which type they are of.");
    case 'drush:sync':
      return dt("Usage: drush [options] sync <source> <destination>\n\nRsync the entire drupal directory or a subdirectory to a <destination> using ssh. Excludes .svn directories. Useful for pushing copies of your tree to a staging server, or retrieving a files directory from a remote site. Local paths should be specified relative to Drupal root.");
    case 'drush:eval':
      return dt("Usage: drush eval <code>\n\nRun arbitrary PHP code in the context of Drupal. For example: drush eval \"variable_set('hello', 'world');\".");
  }
}

/**
 * Implementation of hook_drush_command().
 */
function tools_drush_command() {
  $items['watchdog show'] = array(
    'callback' => 'drush_tools_watchdog_show',
    'description' => 'Show the most recent watchdog log messages.'
  );
  $items['watchdog delete'] = array(
    'callback' => 'drush_tools_watchdog_delete',
    'description' => 'Delete all messages or only those of a specified type.'
  );
  $items['sync'] = array(
    'callback' => 'drush_tools_sync',
    'description' => 'Rsync the Drupal tree to/from another server using ssh.'
  );
  $items['eval'] = array(
    'callback' => 'drush_tools_eval',
    'description' => 'Evaluate arbitrary php code.',
  );
  return $items;
}

/**
 * Push files from or to the local Drupal install using SSH and RSync
 *
 * @return void
 **/
function drush_tools_sync($source, $destination) {
  // Local paths are relative to Drupal root
  if (!strstr($source, ':')) {
    $source = DRUSH_DRUPAL_ROOT. "/$source";
  }
  if (!strstr($destination, ':')) {
    $destination = DRUSH_DRUPAL_ROOT. "/$destination";
  }

  // Prompt for confirmation. This is destructive.
  if (!DRUSH_SIMULATE) {
    drush_print(dt("You will destroy data from !target and replace with data from !source", array('!source' => $source, '!target' => $destination)));
    if (!drush_confirm(dt('Do you really want to continue?'))) {
      drush_die('Aborting.');
    }
  }

  $options = '-az';
  $exec = "rsync -e ssh $options --exclude \"*.svn*\" $source $destination";
  if (DRUSH_VERBOSE) {
    // the drush_op() will be verbose about the command that gets executed.
    $options .= 'v';
  }

  return drush_op('system', $exec) !== FALSE;
}

/**
 * Displays the most recent watchdog log messages (default: 10 messages).
 */
function drush_tools_watchdog_show($limit = 10, $type = NULL) {
  $severities = array(WATCHDOG_NOTICE => dt('notice'), WATCHDOG_WARNING => dt('warning'), WATCHDOG_ERROR => dt('error'));

  $sql = 'SELECT w.*, u.name, u.uid FROM {watchdog} w INNER JOIN {users} u ON w.uid = u.uid ORDER BY w.wid DESC';
  $result = (!empty($type) ?
             db_query_range($sql . " WHERE w.type = '%s'", $type, 0, (int)$limit) :
             db_query_range($sql, 0, (int)$limit));


  $rows = array();
  module_load_include('inc', 'dblog', 'dblog.admin');
  while ($watchdog = db_fetch_object($result)) {
    $rows[] = array(
      format_date($watchdog->timestamp, 'small'),
      $severities[$watchdog->severity],
      dt($watchdog->type),
      truncate_utf8(decode_entities(strtr($watchdog->message, unserialize($watchdog->variables))), 68, FALSE, FALSE),
      theme('username', $watchdog),
    );
  }

  if (count($rows) == 0) {
    drush_die(dt('No log messages available.'));
  }
  drush_verbose(dt('Last !count watchdog log messages:', array('!count' => $limit)));

  array_unshift($rows, array(dt('Date'), dt('Severity'), dt('Type'), dt('Message'), dt('User')));
  drush_print_table($rows, 2, TRUE);
}

/**
 * Deletes all log messages of a certain type from the watchdog log
 * (default: all).
 */
function drush_tools_watchdog_delete($type = NULL) {
  if ($type == "all") {
    drush_op('db_query', 'DELETE FROM {watchdog}'); // indiscriminately delete all
    drush_print(dt('Deleted !n rows.', array('!n' => db_affected_rows())));
  }
  elseif (!empty($type)) {
    drush_op('db_query', 'DELETE FROM {watchdog} WHERE type = \'%s\'', $type);
    drush_print(dt('Deleted !n rows.', array('!n' => db_affected_rows())));
  }
  else {
    drush_print(dt('Please specify a message type, or "all" to delete all messages.'));
  }
}

function drush_tools_eval($command) {
  eval($command);
}
