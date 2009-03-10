<?php

class eaccCache extends Cache {
  /**
   * page_fast_cache
   *   This tells CacheRouter to use page_fast_cache.
   *
   *   @return bool TRUE
   */
  function page_fast_cache() {
    return $this->fast_cache;
  }

  /**
   * get()
   *   Return item from cache if it is available.
   *
   * @param string $key
   *   The key to fetch.
   * @return mixed object|bool
   *   Returns either the cache object or FALSE on failure
   */
  function get($key) {
    $cache = parent::get($this->key($key));
    if (isset($cache)) {
      return $cache;
    }

    $cache = eaccelerator_get($this->key($key));
    if (!empty($cache)) {
      $cache = unserialize($cache);
    }
    parent::set($this->key($key), $cache);
    return $cache;
  }

  /**
   * set()
   *   Add item into cache.
   *
   * @param string $key
   *   The key to set.
   * @param string $value
   *   The data to store in the cache.
   * @param string $expire
   *   The time to expire in seconds.
   * @param string $headers
   *   The page headers.
   * @return bool
   *   Returns TRUE on success or FALSE on failure
   */
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

    if (!is_string($value)) {
      $cache->serialized = TRUE;
      $cache->data = serialize($value);
    }
    else { 
      $cache->serialized = FALSE;
      $cache->data = $value;
    }

    if (!empty($key) && $this->lock()) {
      // Get lookup table to be able to keep track of bins
      $lookup = eaccelerator_get($this->lookup);

      // If the lookup table is empty, initialize table
      if (empty($lookup)) {
        $lookup = array();
      }

      // Set key to 1 so we can keep track of the bin
      $lookup[$this->key($key)] = 1;

      // Attempt to store full key and value
      if (!eaccelerator_put($this->key($key), $cache, $expire)) {
        unset($lookup[$this->key($key)]);
        $return = FALSE;
      }
      else {
        // Update static cache
        parent::set($this->key($key), $cache);
        $return = TRUE;
      }
      
      // Resave the lookup table (even on failure)
      eaccelerator_put($this->lookup, $lookup, 0);

      // Remove lock.
      $this->unlock();
    }

    return $return;

  /**
   * delete()
   *   Remove item from cache.
   *
   * @param string $key
   *   The key to delete.
   * @return mixed object|bool
   *   Returns either the cache object or FALSE on failure
   */
  function delete($key) {
    // Remove from static array cache.
    parent::flush();

    if (substr($key, -1, 1) == '*') {
      if ($this->lock()) {
        $lookup = eaccelerator_get($this->lookup);
        if (!is_null($lookup)) {
        	$lookup = unserialize($lookup);
        }
        if (is_array($lookup)) {
          if ($key == '*') {
            //Fast clean of lookup
            eaccelerator_put($this->lookup, array());
            $this->unlock();
            foreach ($lookup as $k => $v) {
              eaccelerator_rm($k);
            }
            return TRUE;
          }
          else {
            $key = substr($key, 0, strlen($key) - 1);
            foreach ($lookup as $k => $v) {
              if (strpos($k, $key) === 0) {
              //if (substr($k, 0, strlen($key) - 1)) {
                eaccelerator_rm($k);
                unset($lookup[$k]);
              }
            }
          }
        }
        else {
        	$lookup = array();
        }
        eaccelerator_put($this->lookup, serialize($lookup));
        $this->unlock();
      }
    }
    else {
      //Remove only key - clean $lookup on flush
      return eaccelerator_rm($this->key($key));
    }
  }

  /**
   * flush()
   *   Flush the entire cache.
   *
   * @param none
   * @return mixed bool
   *   Returns TRUE
   */
  function flush() {
    parent::flush();
    if ($this->lock()) {
      // Get lookup table to be able to keep track of bins
      $lookup = eaccelerator_get($this->lookup);

      // If the lookup table is empty, remove lock and return
      if (empty($lookup)) {
        $this->unlock();
        return TRUE;
      }
      $lookup = unserialize($lookup);

      // Cycle through keys and remove each entry from the cache
      if (is_array($lookup)) {
        foreach ($lookup as $k => $v) {
          if ($v == CACHE_TEMPORARY || is_null(eaccelerator_get($k))) {
            if (eaccelerator_rm($k)) {
              unset($lookup[$k]);
            }
          }
        }
      }
      else {
      	$lookup = array();
      }

      // Resave the lookup table (even on failure)
      eaccelerator_put($this->lookup, serialize($lookup));

      // Remove lock
      $this->unlock();
      eaccelerator_gc();
    }
    return TRUE;
  }

  /**
   * lock()
   *   lock the cache from other writes.
   *
   * @param none
   * @return string
   *   Returns TRUE on success, FALSE on failure
   */
  function lock() {
    return eaccelerator_lock($this->lock);
  }

  /**
   * unlock()
   *   lock the cache from other writes.
   *
   * @param none
   * @return bool
   *   Returns TRUE on success, FALSE on failure
   */
  function unlock() {
    return  eaccelerator_unlock($this->lock);
  }
}
