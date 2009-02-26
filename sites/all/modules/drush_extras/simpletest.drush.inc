<?php
// $Id: simpletest.drush.inc,v 1.1.2.2 2009/01/16 05:13:09 weitzman Exp $

/**
 * @file Drush simpletest commands
 */

/**
 * Implementation of hook_drush_help().
 */
function simpletest_drush_help($section) {
  switch ($section) {
    case 'drush:test mail':
      return dt("Usage drush [options] test mail <recipients> <extra> <php>.\n\nRun tests and email the results.\n<recipients> should be a comma delimited list of email addresses.\n<extra> contanis any arguments which should be passed along to run-tests.sh. For example, the class of tests to run.\n<php> is rarely needed. Pass the full path to your php interpreter.");
  }
}

/**
 * Implementation of hook_drush_command().
 */
function simpletest_drush_command() {
  $items['test mail'] = array(
    'callback' => 'drush_test_mail',
    'description' => 'Run all tests and mail the results to your team.',
    'bootstrap' => -1,
  );
  
  return $items;
}

// A Drush command callback.
function drush_test_mail() {
  $run_tests = './scripts/run-tests.sh';
  if (!file_exists($run_tests)) {
    drush_error(dt('You must put run-tests.sh in your /scripts directory under Drupal root. You may copy it from http://cvs.drupal.org/viewvc.py/drupal/drupal/scripts/run-tests.sh?revision=1.16&content-type=text%2Fplain'));
  }
  
  if ($recipients = drush_get_option('recipients')) {
    $php = drush_simpletest_find_php();
    $extra = drush_get_option('extra');
    $url = escapeshellarg(url('', array('absolute' => TRUE)));
    $exec = $php . " $run_tests --php '" . $php . '\' --url ' . $url . " $extra";
    drush_shell_exec($exec);
    $output =  implode("\n", drush_shell_exec_output());
    $subject = 'Simpletest results - ' . drush_simpletest_format_results($output);
    mail($recipients, $subject, $output);
  }
}

// Copied from run-tests.sh
function drush_simpletest_find_php() {
  // Determine location of php command automatically, unless a comamnd line argument is supplied.
  if (!$php = drush_get_option('php')) {
    if (!empty($_ENV['_'])) {
      // '_' is an environment variable set by the shell. It contains the command that was executed.
      $php = $_ENV['_'];
    }
    elseif (!empty($_ENV['SUDO_COMMAND'])) {
      // 'SUDO_COMMAND' is an environment variable set by the sudo program.
      // Extract only the PHP interpreter, not the rest of the command.
      list($php, ) = explode(' ', $_ENV['SUDO_COMMAND'], 2);
    }
    else {
      drush_error(dt('Unable to automatically determine the path to the PHP interpreter. Please supply the --php argument.'));
    }
  }
  return $php;
}

// Based on pifr_review_format_results().
function drush_simpletest_format_results($output) {
  if (preg_match_all('/^(.*?) (\d+) (?:pass|passes), (\d+) fails?, and (\d+) exceptions?$/m', $output, $matches, PREG_SET_ORDER)) {
    $results = array('pass' => 0, 'fail' => 0, 'exception' => 0);
    foreach ($matches as $match) {
      // Add to results count array.
      $results['pass'] += $match[2];
      $results['fail'] += $match[3];
      $results['exception'] += $match[4];
    }
    return $results['pass']  . ' pass, ' . $results['fail'] . ' failures, ' . $results['exceptions'] . '  exceptions';
  }
  else {
    return dt('Unknown.');
  }
}
