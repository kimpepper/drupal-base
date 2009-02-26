// $Id
$(document).ready(function(){
  // jQuery Suckerfish Script, Myles Angell
  // http://be.twixt.us/jquery/suckerFish.php
  $("#navigation ul.menu li").hover(
    function(){ $("ul", this).slideDown("fast"); },
    function() { }
  );
  if (document.all) {
    $("#navigation ul.menu li").hoverClass("sfhover");
  }
  // Copy forum comment links
  $(".copy-comment").click( function() {
    prompt('Link to this comment:', this.href);
   });
   
}); // end doc ready

$.fn.hoverClass = function(c) {
  return this.each(function(){
    $(this).hover( 
      function() { $(this).addClass(c); },
      function() { $(this).removeClass(c); }
    );
  });
};