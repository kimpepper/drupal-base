<?php
require 'Cache.php';

class CacheRouter {
  var $map = array();
  var $settings = array();

  function __construct() {
    global $conf;
    $conf['page_cache_fastpath'] = TRUE;
  }
  
  private function __init($bin) {
    global $conf;
    
    if (isset($conf['cacherouter'][$bin]['engine']) && !isset($this->map[$bin])) {
      $type = strtolower($conf['cacherouter'][$bin]['engine']);
    }
    else {
      $type = isset($conf['cacherouter']['default']['engine']) ? $conf['cacherouter']['default']['engine'] : 'db';
    }
    if (!class_exists($type . 'Cache')) {
      if (!require(dirname(__FILE__) .'/engines/' . $type . '.php')) {
        return FALSE;
      }
    }
    $cache_engine = $type . 'Cache';
    
    $this->map[$bin] =& new $cache_engine($bin);
  }
  
  public function get($key, $bin) {
    if (!isset($this->map[$bin])) {
      $this->__init($bin);
    }
    return $this->map[$bin]->get($key);
  }

  public function set($key, $value, $expire, $headers, $bin) {
    if (!isset($this->map[$bin])) {
      $this->__init($bin);
    }
    return $this->map[$bin]->set($key, $value, $expire, $headers);
  }

  public function delete($key, $bin) {
    if (!isset($this->map[$bin])) {
      $this->__init($bin);
    }
    return $this->map[$bin]->delete($key);
  }

  public function flush($bin) {
    if (!isset($this->map[$bin])) {
      $this->__init($bin);
    }
    return $this->map[$bin]->flush();
  }
  
  public function page_fast_cache($bin) {
    if (!isset($this->map[$bin])) {
      $this->__init($bin);
    }
    return $this->map[$bin]->page_fast_cache();
  }
}