<?php

require_once "drush_mm.inc";

$g = new digraph();

//$g->add_link( '/dev/', '/etc/blkid');
//$g->add_link( '/etc/fstab', '/etc/blkid');
$g->add_link( 'a', 'b');
$g->add_link( 'd', 'e');
$g->add_link( 'a', 'd');
$g->add_link( 'b', 'e');
$g->add_link( 'b', 'c');
// a b e
//     c
//   d 
// introduce cyclic link
//$g->add_link( 'b', 'a');

//$g->logger= new logger();

//print_r( $g);

echo "DFS " . print_r($g->tsl());


/**
 * a graph is a set of nodes connected to others
 *
 * by maintaining a list of lists connections can be made
 * ['a']['b'] means 'a' connects to 'b'
 */
class digraph {
  var $_nodes = array();

  /**
   * Topological Sorted List
   *
   * A topological list can only be generated from a acylic directed graph.
   * @param $from_node_key
   *   starting point for sorting from
   * @param $to_node_key
   *   end point to sort to
   */
  function tsl(){
    isset( $this->logger) ? $this->logger->log( "tls.enter") : null ;

    // TODO: for now assume graph is acyclic
    $result= array();
  
    $reset = TRUE;
    foreach( $this->_nodes as $key=>$value){
      isset( $this->logger) ? $this->logger->log( "$key") : null ;
      $result= $this->dfs( $key, $reset);
      $reset= FALSE;
    }

    isset( $this->logger) ? $this->logger->log( "tls.exit") : null ;
    return $result;
  }
  
  function attach( $key, $data){
    if( !array_key_exists($from_node_key, $this->_nodes)){
      $node = new graph_node();
      $node->attach($data);
      $this->_nodes[$from_node_key]= $node;
    }
  }
  
  function add_link( $from_node_key, $to_node_key){
    if( !array_key_exists($from_node_key, $this->_nodes)){
      // add from node
      $this->_nodes[$from_node_key]= new graph_node();
    }
    $node = $this->_nodes[$from_node_key];
    $node->add_link( $to_node_key);
  }
  
  /**
   *  Depth First Search
   */
  function dfs( $node, $reset=false){
    isset( $this->logger) ? $this->logger->log( "dfs.enter") : null;

    static $visited;
    static $result;

    if( !isset( $visited) || $reset){
      $visited= array();
      $result= array();
    }
  
    if( !isset($visited[$node])){
      $visited[$node]=TRUE;
      foreach( $this->_nodes as $key=> $_node){
        $this->dfs( $key);
      }
      array_push($result, $node);
    }
  
    isset( $this->logger) ? $this->logger->log( "dfs.exit") : null;
    return $result;
  }
}

class graph_node {
  var $_key= "";
  var $_links = array();
  var $_data = NULL;
  
  function add_link( $to_node_key){
    $this->_links[$to_node_key]++;
  }
  
  function attach( $data){
    $this->_date = $data;
  }
}

class logger {
  function log($line){
    echo $line ."\n" ;
  }
}

