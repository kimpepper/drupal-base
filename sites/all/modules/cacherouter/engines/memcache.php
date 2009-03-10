<?php
class memcacheCache extends Cache {
  var $settings = array();
  var $memcache;
  
  function page_fast_cache() {
    return $this->fast_cache;
  }
  
  function __construct($bin) {
    global $conf;
    // Assign the servers on the following order: bin specific -> default specific -> localhost port 11211
    if (isset($conf['cacherouter'][$bin]['server'])) {
    	$this->settings['servers'] = $conf['cacherouter'][$bin]['server'];
    	$this->settings['compress'] = isset($conf['cacherouter'][$bin]['compress']) ? MEMCACHE_COMPRESSED : 0;
      $this->settings['shared'] = isset($conf['cacherouter'][$bin]['shared']) ?
                                  $conf['cacherouter'][$bin]['shared'] : TRUE;
    }
    else {
      if (isset($conf['cacherouter']['default']['server'])) {
        $this->settings['servers'] = $conf['cacherouter']['default']['server'];
        $this->settings['compress'] = isset($conf['cacherouter']['default']['compress']) ? MEMCACHE_COMPRESSED : 0;
        $this->settings['shared'] = isset($conf['cacherouter']['default']['shared']) ?
                                    $conf['cacherouter']['default']['shared'] : TRUE;
      }
      else {
        $this->settings['servers'] = array('localhost:11211');
        $this->settings['compress'] = 0;
        $this->settings['shared'] = TRUE;
      }
    }
                                
    parent::__construct($bin);
    
    $this->connect();
  }
  
  function get($key) {
    // Attempt to pull from static cache.
    $cache = parent::get($this->key($key));
    if (isset($cache)) {
      return $cache;
    }
    
    // Get from memcache
    $cache = $this->memcache->get($this->key($key));
    
    // Update static cache 
    parent::set($this->key($key), $cache);
    
    return $cache;
  }
  
  function set($key, $value, $expire = CACHE_PERMANENT, $headers = NULL) {
    if ($expire == CACHE_TEMPORARY) {
      $expire = 180;
    }
    
    // Create new cache object.
    $cache = new stdClass;
    $cache->cid = $key;
    $cache->created = time();
    $cache->expire = $expire;
    $cache->headers = $headers;
    $cache->data = $value;
    
    if (!empty($key)) {
      if ($this->settings['shared']) {
        if ($this->lock()) {
          // Get lookup table to be able to keep track of bins
          $lookup = $this->memcache->get($this->lookup);

          // If the lookup table is empty, initialize table
          if (empty($lookup)) {
            $lookup = array();
          }

          // Set key to 1 so we can keep track of the bin
          $lookup[$this->key($key)] = 1;

          // Attempt to store full key and value
          if (!$this->memcache->set($this->key($key), $cache, $this->settings['compress'], $expire)) {
            unset($lookup[$this->key($key)]);
            $return = FALSE;
          }
          else {
            // Update static cache
            parent::set($this->key($key), $cache);
            $return = TRUE;
          }

          // Resave the lookup table (even on failure)
          $this->memcache->set($this->lookup, $lookup, $this->settings['compress'], $expire);  

          // Remove lock.
          $this->unlock();
        }
      }
      else {
        // Update memcache
        return $this->memcache->set($this->key($key), $cache, $this->settings['compress'], $expire);
      }
    }
  }
  
  function delete($key) {
    // Delete from static cache
    parent::delete($this->key($key));
    
    // Remove from memcache.
    return $this->memcache->delete($this->key($key));

  }
  
  function flush() {
    // Flush static cache
    parent::flush();
    
    // If this is a shared cache, we need to cycle through the lookup table and remove individual
    // items directly
    if ($this->settings['shared']) {
      if ($this->lock()) {
        // Get lookup table to be able to keep track of bins
        $lookup = $this->memcache->get($this->lookup);

        // If the lookup table is empty, remove lock and return
        if (empty($lookup)) {
          $this->unlock();
          return TRUE;
        }

        // Cycle through keys and remove each entry from the cache
        foreach ($lookup as $k => $v) {
          if ($this->memcache->delete($k)) {
            unset($lookup[$k]);
          }
        }

        // Resave the lookup table (even on failure)
        $this->memcache->set($this->lookup, $lookup, $this->settings['compress'], 0);

        // Remove lock
        $this->unlock();
      }
    }
    else {
      // Flush memcache
      return $this->memcache->flush();
    }
  }
  
  function lock() {
    // Lock once by trying to add lock file, if we can't get the lock, we will loop
    // for 3 seconds attempting to get lock.  If we still can't get it at that point,
    // then we give up and return FALSE.
    if ($this->memcache->add($this->lock, $this->settings['compress'], 0) === FALSE) {
      $time = time();
      while ($this->memcache->add($this->lock, $this->settings['compress'], 0) === FALSE) {
        if (time() - $time >= 3) {
          return FALSE;
        }
      }
    }
    return TRUE;
  }
  
  function unlock() {
    return $this->memcache->delete($this->lock);
  }
  
  function connect() {
    $this->memcache =& new Memcache;
    foreach ($this->settings['servers'] as $server) {
      list($host, $port) = explode(':', $server);
      if (!$this->memcache->addServer($host, $port)) {
        drupal_set_message("Unable to connect to memcache server $host:$port");
      }
    }
  }
  
  function close() {
    $this->memcache->close();
  }
}