QuestionIndex=0;
toogle(QuestionIndex);

function response(a){
    toogle(QuestionIndex+=a);
}

function toogle(n){
    var slides=document.getElementsByClassName('questbox');
    var x=document.querySelector('.previous');

    // if(n>=1){
    //     x.className="active";
    // }
    // if(n==0){
    //     x.className="previous";
//     }
//     if(n==slides.length-1){
// document.getElementsByClassName('next').innerHTML='Complete';
//     }
    for(var i=0;i<slides.length;i++){
        slides[i].style.display='none';
    }
slides[QuestionIndex].style.display="block";
}

