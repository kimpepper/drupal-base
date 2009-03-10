<?php
class Cache {
  var $settings = array();
  var $content = array();
  var $prefix = '';
  var $name = '';
  var $lookup = '';
  var $lock = '';
  var $fast_cache = TRUE;
  var $static = FALSE;
  
  function __construct($bin) {
    global $conf;
    
    $this->name = $bin;

    // Setup our prefixes so that we can prefix a particular bin, or if not set use the default prefix.
    if (isset($conf['cacherouter'][$bin]['prefix'])) {
      $this->prefix = $conf['cacherouter'][$bin]['prefix'];
    }
    else if (isset($conf['cacherouter']['default']['prefix'])) {
      $this->prefix = $conf['cacherouter']['default']['prefix'];
    }
    
    // This allows us to turn off fast_cache for cache_page so that we can get anonymous statistics.
    if (isset($conf['cacherouter']['default']['fast_cache'])) {
      $this->fast_cache = $conf['cacherouter']['default']['fast_cache'];
    }
    
    // This allows us to turn off static content caching for modules/bins that are already doing this.
    if (isset($conf['cacherouter'][$bin]['static'])) {
      $this->static = $conf['cacherouter'][$bin]['static'];
    }
    else if (isset($conf['cacherouter']['default']['static'])) {
      $this->static = $conf['cacherouter']['default']['static'];
    }
    
    // Setup our prefixed lookup and lock table names for shared storage.
    $this->lookup = (!empty($this->prefix) ? $this->prefix .'-' : '') .'lookup_'. $bin;
    $this->lock = (!empty($this->prefix) ? $this->prefix .'-' : '') .'lock_'. $bin;
  }
  
  function get($key) {
    if (isset($this->content[$key]) && $this->static) {
      return $this->content[$key];
    }
  }
  
  function set($key, $value) {
    if ($this->static) {
      $this->content[$key] = $value;
    }
  }
  
  function delete($key) {
    if ($this->static) {
      unset($this->content[$key]);
    }
  }
  
  function flush() {
    if ($this->static) {
      $this->content = array();
    }
  }
  
  /**
   * key()
   *   Get the full key of the item
   *
   * @param string $key
   *   The key to set.
   * @return string
   *   Returns the full key of the cache item.
   */
  function key($key) {
    return urlencode((!empty($this->prefix) ? $this->prefix .'-' : '') . $key);
  }
}