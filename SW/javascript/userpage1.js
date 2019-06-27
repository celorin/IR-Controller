
$(function(){
  $('.menu-trigger').on('click',function(){
    if($(this).hasClass('active')){
      $(this).removeClass('active');
      $('nav').removeClass('open');
      $('.overlay').removeClass('open');
    } else {
      $(this).addClass('active');
      $('nav').addClass('open');
      $('.overlay').addClass('open');
    }
  });
  $('.overlay').on('click',function(){
    if($(this).hasClass('open')){
      $(this).removeClass('open');
      $('.menu-trigger').removeClass('active');
      $('nav').removeClass('open');      
    }
  });
  
  $(".btn1").on("click", function(){
    $("#overlay").fadeIn(500);
    setTimeout(function(){
        $("#overlay").fadeOut(500);
        $("form").submit();
    },3000);
  });
  
});