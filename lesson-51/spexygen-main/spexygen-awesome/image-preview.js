/* Image preview script 
 * powered by jQuery (http://www.jquery.com)
 * written by Alen Grakalic (http://cssglobe.com)
 * for more info visit
 * http://cssglobe.com/post/1695/easiest-tooltip-and-image-preview-using-jquery
 */
this.imagePreview = function(){	
  // you might want to adjust to get the right result
  horOffset = -100;
  verOffset = 0;
		
  /* END CONFIG */
  $("a.preview").hover(function(e){
    this.t = this.title;
    this.title = "";	
    var c = (this.t != "") ? "<br/>" + this.t : "";
    $("body").append("<p id='preview'><img src='"+ this.href +"' alt='Image preview' />"+ c +"</p>");								 
    $("#preview")
      .css("top",(e.pageY - verOffset) + "px")
      .css("left",(e.pageX + horOffset) + "px")
      .fadeIn("fast");						
  },
  function(){
    this.title = this.t;	
    $("#preview").remove();
  });	
  $("a.preview").mousemove(function(e){
    $("#preview")
      .css("top",(e.pageY - verOffset) + "px")
      .css("left",(e.pageX + horOffset) + "px");
  });			
};
// starting the script on page load
$(document).ready(function(){
	imagePreview();
});