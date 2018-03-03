//require moment.js

var predicts=[];

class Predict {
  constructor(){
  this.spa="unknown";
  this.type='burst';
  this.burstAlt=0.0;
  this.latitude=0.0;
  this.longitude=0.0;
  this.altitude=0.0;
  this.time="";
  this.marker= null;
  }
  parseCSV(predictCsv) {
     var predict=predictCsv.split(",");
                      this.type=predict[0];
                      this.spa=predict[1].trim();
                      this.burstAlt=predict[2];
                      this.time=predict[3];
                      this.latitude=predict[4];
                      this.longitude=predict[5];
                      this.altitude=predict[6];

  }
  getLastFrameTime(){
   return moment(this.time).format("DD HH:mm:ss");

  }
 
 getId(){
   return this.spa+"_"+this.type+"_"+this.burstAlt;
 }
}


function getPredictData(mymap){
    nocache = "&nocache="+ Math.random() * 1000000;
    var request = new XMLHttpRequest();
    request.onreadystatechange = function(){
        if (this.readyState == 4){
            if (this.status == 200){
                if (this.responseText != null){
                    //document.getElementById("mapmenu").innerHTML = this.responseText;
                    var csvData=this.responseText.split("\n");
                    csvData.splice(-1);
                     menuInner="";

                     //sondeLayer=null;
                     for(line in csvData  ) {
                      //if (line>9) break;
                      var predictPar=new Predict();
                      predictPar.parseCSV(csvData[line]);

                      if(typeof predicts[predictPar.getId()] === 'undefined') {
                           predicts[predictPar.getId()]=predictPar;
                            var iconP=touchdownIcon;
                            if (predictPar.type == 'burst') iconP=burstIcon;
                            predictPar.marker= predictMarker=L.marker([predictPar.latitude, predictPar.longitude],{icon: iconP})
                                 .bindPopup("<b>"+predictPar.spa+"</b><br />"+predictPar.type+" "+predictPar.burstAlt+" "+predictPar.time).addTo(mymap);
                           
                           updatePredictList(predictPar);
                      } //if typeof
                     } //for

                } //if response
            } //if status 200
        }
    }
    request.open("GET", "getcsvpred.php", true);
    request.send(null);
}


function updatePredictList(predictPar){
 var idItem="predict_item_"+predictPar.getId();
 var div = document.getElementById(idItem);

 item= "<div id=\""+idItem+"\">"
     +"<button "
     +"onclick=\"predictOnMapJump('"+predictPar.getId()+"')\">" +predictPar.getId()+"</button>"
     +"</div>";

if (div  === null) {
var newItem=document.createElement("div");
newItem.id=idItem;
newItem.innerHTML=item;
var controlPredict=document.getElementsByClassName('Predict leaflet-control')[0];
//menu.insertBefore(newItem,menu.childNodes[0]);
controlPredict.appendChild(newItem);
}
/*else
{
//sondeDetail.innerHTML=getSondeDetailContent(sondePar);
}
*/

}


function cleanOldPredicts(){

//for (i=predict.length-1;i>=0,i--) {
   //if predict[i].time
//}

}

