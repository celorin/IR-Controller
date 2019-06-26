
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
});

function check(){
  if(window.confirm('送信してよろしいですか？')){ 
    window.location.href = '../html/userpage1.html'; 
		return true;
	}
	else{
		window.alert('キャンセルされました');
		return false;
	}
}