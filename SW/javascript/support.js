

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