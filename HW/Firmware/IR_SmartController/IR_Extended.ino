int getTmp(){
  int analog , tmp, vt;
  analog = analogRead(TMPPin); 
  vt  = map(analog,0,4095,0,4095);    
  tmp = map(vt,300,1600,-30,100); 
  return tmp;
}
