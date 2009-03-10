<?php

class fileCache extends Cache {
  var $content = array();
  var $fspath = '/tmp/filecache';
  
  function page_fast_cache() {
    return $this->fast_cache;
  }
  
  function __construct($bin) {
    global $conf;
    
    // Assign the path on the following order: bin specific -> default specific -> /tmp/filepath
    $this->fspath = $conf['cacherouter'][$bin]['path'];
    if (!isset($conf['cacherouter'][$bin]['path'])) {
      if (isset($conf['cacherouter']['default']['path'])) {
        $this->fspath = $conf['cacherouter']['default']['path'];
      }
    }

    parent::__construct($bin);
  }
  
  function get($key) {
    global $user;
    
    if (isset($this->content[$key])) {
      return $this->content[$key];
    }
    
    $cache_lifetime = variable_get('cache_lifetime', 0);

    if (variable_get('page_cache_fastpath', 0)) {
      $cache = NULL;
      $cache_file = $this->key($key);

      if (file_exists($cache_file)) {    
        if ($fp = fopen($cache_file, 'r')) {
          if (flock($fp, LOCK_SH)) {
            $data = fread($fp, filesize($cache_file));
            flock($fp, LOCK_UN);
            $cache = unserialize($data);
          }
          fclose($fp);
        }
      }
      return $cache;

    }
    else {
      // garbage collection necessary when enforcing a minimum cache lifetime
      $cache_flush = variable_get('cache_flush', 0);
      if ($cache_flush && ($cache_flush + $cache_lifetime <= time())) {
        variable_set('cache_flush', 0);
      }
    }

    if (isset($cache->data)) {
      // If enforcing a minimum cache lifetime, validate that the data is
      // currently valid for this user before we return it by making sure the
      // cache entry was created before the timestamp in the current session's
      // cache timer. The cache variable is loaded into the $user object by
      // sess_read() in session.inc.
      if ($user->cache > $cache->created) {
        // This cache data is too old and thus not valid for us, ignore it.
        return FALSE;
      }
      return $cache;
    }
    return FALSE;
  }
  
  function set($key, $value, $expire = CACHE_PERMANENT, $headers = NULL) {
    static $subdirectories;

    if (variable_get('page_cache_fastpath', 0)) {
      // prepare the cache before grabbing the file lock
      $cache = new stdClass;
      $cache->cid = $key;
      $cache->table = $this->name;
      $cache->created = time();
      $cache->expire = $expire;
      $cache->headers = $headers;
      $cache->data = $value;
      
      $data = serialize($cache);
      
      $file = $this->key($key);
      if ($fp = @fopen($file, 'w')) {
        // only write to the cache file if we can obtain an exclusive lock
        if (flock($fp, LOCK_EX)) {
          fwrite($fp, $data);
          flock($fp, LOCK_UN);
        }
        fclose($fp);
      }
      else {
        // t() may not be loaded
        drupal_set_message(strtr('Cache write error, failed to open file "%file"', array('%file' => $file)), 'error');
      }
    }
  }
  
  function delete($key) {
    // when using wildcard: $key is part-of-key + '*'
    if (strrpos($key, '*')) {
      $look_for = explode('*', $key);
      $fspath = $this->fspath;
      // Filename: abcdef12345verylongmd5code--content:123456:987654
      $files = file_scan_directory($fspath, ".--$look_for[0].", array('.', '..', 'CVS'));
      foreach ($files as $file) {
        if ($fp = fopen($file, 'w')) {
          // only delete the cache file once we obtain an exclusive lock to prevent
          // deleting a cache file that is currently being read.
          if (flock($fp, LOCK_EX)) {
            unlink($file);
          }
        }
      }      
    }
    else {
      $file = $this->key($key);
      if ($fp = fopen($file, 'w')) {
        // only delete the cache file once we obtain an exclusive lock to prevent
        // deleting a cache file that is currently being read.
        if (flock($fp, LOCK_EX)) {
          unlink($file);
        }
      }
    }
  }
  
  function flush() {
    global $user;
    
    $table = $this->name;
    $cache_lifetime = variable_get('cache_lifetime', 0);
    $fspath = $this->fspath;
    
    $ivalfrom = ord("a");
    $ivalto = ord("f");
    
    for ($i = $ivalfrom; $i <= $ivalto; $i++) {
      $this->purge("$fspath/$table/". chr($i));
    }

    for ($i = 0; $i<10; $i++) {
      $this->purge("$fspath/$table/". $i);
    }
  }
  
  function key($key) {
    $table = $this->name;
    $fspath = $this->fspath;
    $hash = md5($key);
    // TODO make sure we always get valid filenames in the appendix
    $appendix = str_replace(array('/'), array('-'), $key);
    
    $this->create_directory($fspath, $hash{0});
    
    return  "$fspath/$table/". $hash{0}. '/'. $hash. '--'. $appendix;
  }
  
  /**
   * Create the necessary $table directory and/or letter/number subdirectory if
   * it doesn't exist.  We store the directories we've created in a static 
   * so we don't bother doing an fstat on that more than one time per page load.
   */
  function create_directory($fspath, $hash) {
    static $dirs = array();
    $table = $this->name;
    
    $create = array($fspath, "$fspath/$table", "$fspath/$table/$hash");

    foreach ($create as $dir) {
      $dir = rtrim($dir, '/\\');
      // Check the static $dirs to avoid excessive fstats
      if (!isset($dirs[$dir])) {
        $dirs[$dir] = 1;
        if (!is_dir($dir)) {
          if (!mkdir($dir)) {
            // t() is not available.
            drupal_set_message(strtr('Failed to create directory %dir.', array('%dir' => "<em>$dir</em>")));
          }
          else {
            @chmod($subdirectory, 0775); // Necessary for non-webserver users.
          }
        }
      }
    }
  }

  function purge($dir) {
    global $cache_lifetime;
    $files = file_scan_directory($dir, '.', array('.', '..', 'CVS'));
    foreach ($files as $file) {
      if (filemtime($file->filename) < (time() - $cache_lifetime)) {
        if ($fp = fopen($file->filename, 'r')) {
          // We need an exclusive lock, but don't block if we can't get it as
          // we can simply try again next time cron is run.
          if (flock($fp, LOCK_EX|LOCK_NB)) {
            unlink($file->filename);
          }
        }
      }
    }
  }
}
