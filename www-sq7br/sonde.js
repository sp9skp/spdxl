//require moment.js

var activeSondeId='';


function followSonde(mymap){
      if ( activeSondeId != null ) {
            sondesParams[sondeId].mapJump(mymap);
            sondesParams[sondeId].marker.openPopup();
      };
}

class SPath{
        constructor(lat,lon,alt,speed,climb,direct,time){
          this.latitude=lat;
          this.longitude=lon;
          this.altitude=alt;
          this.speed=speed;
          this.climb=climb;
          this.direction=direct;
          this.time=time;
         }
}

class SondeParser{
        constructor(sondeCSV){
            var sonde=sondeCSV.split(";");
               this.Id=sonde[0];
               this.latitude=sonde[1]; //lat
               this.longitude=sonde[2]; //lon
               this.altitude=sonde[3]; //alt
               this.speed=sonde[4]; //speed
               this.climb=sonde[5]; //climb
               this.direction=sonde[6]; //direct
               this.frequency=sonde[7];
               this.time=sonde[8];
       }


}


class Sonde {
  constructor(sondeParser,map){
    this.map=map;
    this.lastFrameTime=0;
    this.polyline = new L.Polyline([], {
                             color: 'red',
                             weight: 5,
                             smoothFactor: 1
                             }).addTo(map);

    this.icon = new L.Icon(
                                {
                                 iconUrl: 'ico/balon3.png'
                                ,iconSize: [30, 30]
                                ,shadowUrl:'ico/balon_cien2.png'
                                ,shadowSize: [30,30]
                                ,iconAnchor:   [15, 30]
                                ,shadowAnchor: [0, 30]
                                ,popupAnchor:  [0, 0]
                                }
                            );
    this.marker = new L.marker([0.0, 0.0],{icon:this.icon}).addTo(map);
    this.path=[];
    //alert("sonde:"+ sondeParser.time);
    this.copyPosition(sondeParser);
  }
  copyPosition(sondeParser) {
       this.Id=sondeParser.Id.trim();
       if ( this.lastFrameTime<sondeParser.time ) {
                      this.lastFrameTime=sondeParser.time;
                      this.frequency=sondeParser.frequency;
                      this.path[this.lastFrameTime]=new SPath(
                      sondeParser.latitude 
                      ,sondeParser.longitude
                      ,sondeParser.altitude
                      ,sondeParser.speed
                      ,sondeParser.climb
                      ,sondeParser.direction
                      );
                      this.polyline.addLatLng(new L.LatLng(sondeParser.latitude,sondeParser.longitude));
                      this.marker.setLatLng([sondeParser.latitude, sondeParser.longitude]);
                      var baloonHigh=Math.round( (sondeParser.altitude/1000) *20 );
                      //this.marker.icon.iconAnchor=[15 , 30+baloonHigh];
                      //this.marker.icon=this.icon;
                      //alert(this.marker );
                      this.marker.bindPopup(this.getSondeDetailContent());
                    }
  }
  getLastFrameTime(){
   return moment.unix(this.lastFrameTime).format("DD HH:mm:ss");

  }

getSondeDetailContent(){
 var lastPath=this.path[this.lastFrameTime];
 if (lastPath == null) return "no data";
 var  sondeDetailContent=lastPath.latitude+" "+lastPath.longitude
     +"<br>"+this.Id
     +" Time: "+this.getLastFrameTime()
     +"<br> Climb: "+lastPath.climb
     +" Alt: "+lastPath.altitude+" m."
     +"<br> Dir: "+lastPath.direction+" st."
     +" Speed: "+lastPath.speed+" m/s"
     +"<br> Freq: "+this.frequency+" MHz"
     +"";

return sondeDetailContent;
}

mapJump(map){
   map.panTo([this.path[this.lastFrameTime].latitude,this.path[this.lastFrameTime].longitude]);
}

}


function myOverSondeFunction(id){
var divSonde = document.getElementById(id);
divSonde.style.display="block";
}

function myOverSondeOutFunction(id){
var divSonde = document.getElementById(id);
divSonde.style.display="none";
}

function toggleDiv(id) {
    var div = document.getElementById(id);
    div.style.display = div.style.display == "none" ? "block" : "none";
}
