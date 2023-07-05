
var slideIndex=0;
moveIt(slideIndex);

function jump(a){
    moveIt(slideIndex=a);
}
function movement(m){
    moveIt(slideIndex+=m);
}


function moveIt(n){
var slides=document.getElementsByClassName('content');

var images=document.getElementsByClassName('down');

if(n==-1){slideIndex=slides.length-1;}
if(n==slides.length){slideIndex=0;}

for(var i=0;i<slides.length;i++){
    slides[i].style.display="none";
}
for(var i=0;i<images.length;i++){
   images[i].className = images[i].className.replace(" active","");
}

images[slideIndex].className+=" active";
slides[slideIndex].style.display='block';
}
// var slides=document.getElementsByClassName('content');
// console.log(slides[2].id);
// console.log(slides.length);