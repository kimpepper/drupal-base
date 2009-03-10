<?php
class dbCache extends Cache {
  var $content = array();
  
  function page_fast_cache() {
    return FALSE;
  }
  
  function get($key) {
    global $user;
    
    // Load local cache for multiple hits per page.
    if (isset($this->content[$key])) {
      return $this->content[$key];
    }
    
    // Handle garbage collection
    $this->gc();
    
    $cache = db_fetch_object(db_query("SELECT data, created, headers, expire, serialized FROM {". $this->name ."} WHERE cid = '%s'", $key));
    if (isset($cache->data)) {
      // If the data is permanent or we're not enforcing a minimum cache lifetime
      // always return the cached data.
      if ($cache->expire == CACHE_PERMANENT || !variable_get('cache_lifetime', 0)) {
        $cache->data = db_decode_blob($cache->data);
        if ($cache->serialized) {
          $cache->data = unserialize($cache->data);
        }
      }
      // If enforcing a minimum cache lifetime, validate that the data is
      // currently valid for this user before we return it by making sure the
      // cache entry was created before the timestamp in the current session's
      // cache timer. The cache variable is loaded into the $user object by
      // sess_read() in session.inc.
      else {
        if ($user->cache > $cache->created) {
          // This cache data is too old and thus not valid for us, ignore it.
          return 0;
        }
        else {
          $cache->data = db_decode_blob($cache->data);
          if ($cache->serialized) {
            $cache->data = unserialize($cache->data);
          }
        }
      }
      $this->content[$key] = $cache;
      return $cache;
    }
    return 0;
  }
  
  function set($key, $value, $expire = CACHE_PERMANENT, $headers = NULL) {
    unset($this->content[$key]);
    $serialized = 0;
    if (!is_string($value)) {
      $value = serialize($value);
      $serialized = 1;
    }
    $created = time();
    db_query("UPDATE {". $this->name ."} SET data = %b, created = %d, expire = %d, headers = '%s', serialized = %d WHERE cid = '%s'", $value, $created, $expire, $headers, $serialized, $key);
    if (!db_affected_rows()) {
      @db_query("INSERT INTO {". $this->name ."} (cid, data, created, expire, headers, serialized) VALUES ('%s', %b, %d, %d, '%s', %d)", $key, $value, $created, $expire, $headers, $serialized);
    }
  }
  
  function delete($key) {
    unset($this->content[$key]);
    if (substr($key, -1, 1) == '*') {
      if ($key == '*') {
        db_query("DELETE FROM {". $this->name ."}");
      }
      else {
        $key = substr($key, 0, strlen($key) - 1);
        db_query("DELETE FROM {". $this->name ."} WHERE cid LIKE '%s%%'", $key);
      }
    }
    else {
      db_query("DELETE FROM {". $this->name ."} WHERE cid = '%s'", $key);
    }
  }

  function flush($flush) {
    $flush = empty($flush) ? time() : $flush;
    $this->content = array();
    db_query("DELETE FROM {". $this->name ."} WHERE expire != %d AND expire < %d", CACHE_PERMANENT, $flush);
  }
  
  function gc() {
    // Garbage collection necessary when enforcing a minimum cache lifetime
    $cache_flush = variable_get('cache_flush', 0);
    if ($cache_flush && ($cache_flush + variable_get('cache_lifetime', 0) <= time())) {
      // Reset the variable immediately to prevent a meltdown in heavy load situations.
      variable_set('cache_flush', 0);
      // Time to flush old cache data
      $this->flush($cache_flush);
    }
  }
}