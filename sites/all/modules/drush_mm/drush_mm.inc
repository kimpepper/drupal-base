<?php
// $Id: drush_mm.inc,v 1.5 2009/01/07 12:24:17 clemenstolboom Exp $

/**
 * Get all folder from the given directory
 *
 * from php.net
 * - getRecursiveFolderList
 */
function _drush_mm_get_recursive_folder_list($cur_dir, $current_array=array()) {
  //TODO: file_scan_directory @ includes/file.inc
  //TODO: using DIR_ONLY misses symlinks
  $dirs = glob($cur_dir .'/*');    
  $cur = 0;
  foreach ($dirs as $dir) {
    $current_array[$cur]['path'] = $dir;
    $current_array[$cur] = _drush_mm_get_recursive_folder_list($dir, $current_array[$cur]);
    ++$cur;
  }
  return $current_array;
}

/**
 * Callback to collect info from each *.info file found
 *
 * Each info file is parsed and stored into a module_info array
 * Some fields are used for calculating the dependencies graph
 * Other fields are nice for generating the dot output.
 */
function _drush_mm_get_module_info($item, $key) {
  global $_drush_mm_module_info;
  
  foreach (glob( $item .'/*.info') as $file) {
    $base_name= basename( $file, '.info');
    
    // check later whether module is encountered more then once
    $module_info[ $base_name ]['count']++;

    // nice to know to recalculate the original package
    $module_info[ $base_name ]['dir']= dirname( $file);

    $_drush_mm_module_info[ $base_name]= drupal_parse_info_file( $file);
    $_drush_mm_module_info[ $base_name]['dependencies']= $_drush_mm_module_info[ $base_name]['dependencies'];
  }
}

function _drush_mm_list_modules( $modules_info, $return_list=FALSE) {
  $list = array();
  
  foreach ($modules_info as $key => $value) {
    if ($value['name']) {
      if (DRUSH_VERBOSE) {
        $list[ $value['status']][]=  $key .' ('. $value['version'] .')';
      }
      else {
        $list[ $value['status']][]=  $key;
      }
    } 
    else {
      if (isset($value['status'])) {
        $list['error'][]= $key;
      }
      else {
        $list['missing'][]= $key;
      }
    }
  }
  
  sort( $list['error']);
  sort( $list['missing']);
  sort( $list[0]);
  sort( $list[1]);
  if ($return_list) return $list;
  if (count($list['error'])) {
    echo "According to the system table available modules but not installed!\n";
    echo "  ERROR: ". implode( ", ", $list['error']) ."\n\n";
  }

  if (count($list['missing'])) {
    echo "missing: ". implode( " ", $list['missing']) ."\n\n";
  }
  echo "enabled: ". implode( " ", $list[1]) ."\n\n";
  echo "disabled: ". implode( " ", $list[0]) ."\n\n";
  
}

function _drush_mm_generate_dot( $modules_info) {
  // print_r( $modules_info);
  echo "digraph {\n";
  
  // make this a two pass
  // 1. nodes
  // 1.1 use subgraphs for same package based modules
  // 1.2 use colors 
  // 2. connections
  foreach ($modules_info as $key => $value) {
    echo '  "'. $key .'" [ style=filled, color='
      . ( isset( $value['name']) ? ($value['status'] ? 'green' : 'yellow') : 'red')
      ." ];\n";
      
    if (isset( $value['dependencies'])) {
      foreach ($value['dependencies'] as $dep) {
        echo "  \"$key\" -> \"$dep\" \n";
      }
    } 
    else {
      echo "  \"$key\" \n";
    }
  }
  echo "}";
}

function _drush_mm_get_modules_info() {
  module_rebuild_cache();

  // storage for all module info
  global $_drush_mm_module_info;
  $_drush_mm_module_info= array();

  foreach ($paths= _drush_mm_get_module_paths() as $path) {
    $tree = _drush_mm_get_recursive_folder_list( $path);
    array_walk_recursive( $tree, '_drush_mm_get_module_info');
  }
  
  // TODO: add the system settings
  $result= db_query("SELECT name, status FROM {system} WHERE type = 'module'");
  while ($module= db_fetch_object($result)) {
    $_drush_mm_module_info[$module->name]['status']= $module->status;
  }
  
  // fill in the rdependencies
  foreach ($_drush_mm_module_info as $key => $module) {
    if (is_array($module['dependencies'])) {
      foreach ($module['dependencies'] as $depend) {
        // registrer the reverse dependencies
        $_drush_mm_module_info[$depend]['rdependencies'][]= $key;
      }
    }
  }
}

/**
 * Based on the modules at hand the list of depending modules is generated
 * 
 * Depending on the direction of the request $enable the
 * list is build up on the dependencies (build up) 
 * or the rdependencies (tear down)
 * 
 * @param $modules modules given by the user
 * @param $enable build or teardown the list
 * 
 */
function _drush_mm_get_module_list( $modules, $enable) {
  global $_drush_mm_module_info;
  _drush_mm_get_modules_info();
  drush_verbose(t("Given modules: '!modules'" , array( "!modules" => implode( ", ", $modules))));
  $modules_to_use = array();
  foreach ($modules as $module) {
    drush_verbose("Adding '$module'", 2);
    $modules_to_use[$module]= $_drush_mm_module_info[$module];
  }
  
  $dependency_key = $enable ? 'dependencies' : 'rdependencies';
  
  drush_verbose("Adding $dependency_key to list");
  // add dependencies
  $more= TRUE;
  while ($more) {
    $more= FALSE;
    foreach ($modules_to_use as $key => $module) {
      // add all (r)dependencies to the list
      if (is_array($module[ $dependency_key])) {
        foreach ($module[ $dependency_key] as $depend) {
          if (!array_key_exists( $depend, $modules_to_use)) {
            // we found one so maybe there are more
            $more= TRUE;
            
            drush_verbose("Adding '$depend' as a $dependency_key", 2);
            $modules_to_use[$depend]= $_drush_mm_module_info[$depend];
          }
        }
      }
    }
  }

  // module names added by a rdepend may not exists
  $missing_modules= array();
  foreach ($modules_to_use as $module => $values) {
    // TODO: missing name implies module is missing :-/
    if (!isset( $values['name'])) {
      $missing_modules[] = $module;
    }
  }
  
  if (count( $missing_modules)) {
    // maybe a mistype
    $did_you_mean= array();
    $module_names = array_keys( $_drush_mm_module_info);

    drush_verbose( 'missing modules found ... building suggestions');
    foreach ($missing_modules as $missing_module) {
      drush_verbose( "testing $missing_module ..", 1);
      foreach ($module_names as $existing_module) {
        $match = similar_text($missing_module, $existing_module);

        if ($match> strlen($missing_module)-2) {
          drush_verbose( "similarity to $existing_module .. $match", 2);
          $did_you_mean[$missing_module] .= " ". $existing_module;
        }
      }
    }
    
    if (count($did_you_mean)) {
      $suggestion = t('Did you mean: ') . implode("\n", $did_you_mean);
    }
    else {
      $suggestion = t('No suggestions found!');
    }
    
    drush_die(t("The following modules where not found. '!modules'\n  !suggestion" ,
     array( "!suggestion" => $suggestion, "!modules" => implode( ", ", $missing_modules))));
  }  
  
  $graph= _drush_mm_build_graph( $modules_to_use, $dependency_key);
  
  $result= _drush_mm_graph_tsl( $graph);
  
  if (array_shift($result) != "_drupal_") {
    drush_die(t("Unexpected result for '!modules'." , array( "!modules" => implode( ", ", $result))));
  }
  if (array_pop($result) != "_root_") {
    drush_die(t("Unexpected result for '!modules'." , array( "!modules" => implode( ", ", $result))));
  }
  
  return $result;
}

function _drush_mm_build_graph( $modules_to_use, $dependency_key) {
  // build graph
  $graph= array();
  foreach ($modules_to_use as $key => $module) {
    // make undependants dependant
    if (count($module[ $dependency_key])) {
      foreach ($module[$dependency_key] as $depend) {
        $graph[$key][$depend]++;
      }
    } 
    else {
      // we make the independant 'drupal' dependent
      $graph[$key]['_drupal_']++;
    }
  }

  // make all rdepend-less rdepend to _root_
  foreach ($graph as $key => $node) {
    if (!count( $modules_to_use[$key]['rdepend'])) {
      $graph['_root_'][$key]++;
    }
  }
  return $graph;
}

function _drush_mm_get_module_paths() {
  $paths = array();
  
  if (DRUSH_URI) {
    $path = conf_path();
    $paths['site'] = DRUSH_DRUPAL_ROOT .'/'. $path .'/modules/';
  }
  
  if (file_exists(DRUSH_DRUPAL_ROOT .'/sites/all/modules/')) {
    $paths['all'] = DRUSH_DRUPAL_ROOT .'/sites/all/modules/';
  }
  
  $paths['core'] = DRUSH_DRUPAL_ROOT .'/modules/';
  
  return $paths;
}

/**
 * Generares a topological sorted list
 * 
 * @param $graph
 *   the graph to get a TSL from
 */
function _drush_mm_graph_tsl( $graph) {
  $result= array();
  
  $reset = TRUE;
  foreach ($graph as $key => $value) {
    $result= _drush_mm_graph_df($graph, $key, $reset);
    $reset= FALSE;
  }

  return $result;
}

/**
 * do a depth first search
 * 
 * Each node will be visited. The more depth first.
 */
function _drush_mm_graph_df( $graph, $node, $reset=false) {
  static $visited;
  static $result;
  
  if (!isset( $visited) || $reset) {
    $visited= array();
    $result= array();
  }
  
  if (!isset($visited[$node])) {
    $visited[$node]=TRUE;
    if (is_array($graph[$node])) {
      foreach ($graph[$node] as $key => $_node) {
        _drush_mm_graph_df( $graph, $key);
      }
    }
    array_push($result, $node);
  }
  
  return $result;
}
