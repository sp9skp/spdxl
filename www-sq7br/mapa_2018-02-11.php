<!DOCTYPE html>
<html>
<head>

	<title>Sonde Radar by SQ7BR</title>

	<meta charset="utf-8" />
	<meta name="viewport" content="width=device-width, initial-scale=1.0">

	<link rel="shortcut icon" type="image/x-icon" href="docs/images/favicon.ico" />

<link rel="stylesheet" href="node_modules/leaflet/dist/leaflet.css" />
<link rel="stylesheet" href="styl.css" />

<script src="node_modules/leaflet/dist/leaflet.js"></script>
<script src="node_modules/leaflet-providers/leaflet-providers.js"></script>


<script src="node_modules/leaflet.coordinates/dist/Leaflet.Coordinates-0.1.5.src.js"></script>

<script src="map_icons.js"></script>

<script src="node_modules/moment/moment.js"></script>
<script src="sonde.js"></script>
<script src="predict.js"></script>
<script src="clipboard.js"></script>


<script>
function getData(){
   try {
     getMapData(mymap);
     getPredictData(mymap);
     followSonde(mymap);
   }catch(err) {
      };
    setTimeout('getData()', 1000);
}



</script>
</head>
<body onload="getData()">
<div id="mapid"></div>
<script>
var sondesParams=[];
var polylines=[];
var sondesMarker=[];
</script>

<script>

	var mymap = L.map('mapid').setView([<?php include "config.php"; echo "$lat,$lon"; ?>], 13);

	var defaultLayer=L.tileLayer('https://api.tiles.mapbox.com/v4/{id}/{z}/{x}/{y}.png?access_token=pk.eyJ1IjoibWFwYm94IiwiYSI6ImNpejY4NXVycTA2emYycXBndHRqcmZ3N3gifQ.rJcFIG214AriISLbB6B5aw', {
		maxZoom: 18,
		attribution: 'Map data &copy; <a href="http://openstreetmap.org">OpenStreetMap</a> contributors, ' +
			'<a href="http://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a>, ' +
			'Imagery © <a href="http://mapbox.com">Mapbox</a>',
		id: 'mapbox.streets'
	}).addTo(mymap);

        var OpenTopoMap = L.tileLayer('https://{s}.tile.opentopomap.org/{z}/{x}/{y}.png', {
	    maxZoom: 17,
	    attribution: 'Map data: &copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>, <a href="http://viewfinderpanoramas.org">SRTM</a> | Map style: &copy; <a href="https://opentopomap.org">OpenTopoMap</a> (<a href="https://creativecommons.org/licenses/by-sa/3.0/">CC-BY-SA</a>)'
        });
        var Esri_WorldImagery = L.tileLayer('https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
	   // attribution: 'Tiles &copy; Esri Src: Esri, i-cubed, USDA, USGS, AEX, GeoEye, Getmapping, Aerogrid, IGN, IGP, UPR-EGP, GIS Usr Comity'
        });
           Esri_WorldImagery.setOpacity(0.9);
            //defaultLayer.setOpacity(0.5);

          //ctrLayer = L.control.activeLayers(baseMaps, overlayMaps, {position: 'topright'}).addTo(map);

          //tilemapLayer = ctrLayer.getActiveBaseLayer().layer;
          //tilemapLayer.setOpacity(actualOpacityValue);


      //coordinates info

           L.control.coordinates({
                                   position:"topleft" //optional default "topright"
                                  ,decimals:5 //optional default 4
                                  ,decimalSeperator:"." //optional default "."
                                  ,labelTemplateLat:"{y}" //optional default "Lat: {y}"
                                  ,labelTemplateLng:"{x}" //optional default "Lng: {x}"
                                  ,enableUserInput:false //optional default true
                                  ,useDMS:false //optional default false
                                  ,useLatLngOrder: true //ordering of labels, default false-> lng-lat
                                  //,markerType: L.marker //optional default L.marker
                                  //,markerProps: {} //optional default {},
                                }).addTo(mymap);


       var baseLayers= {
                        'OpenStreetMap Default': defaultLayer
                       ,'Topo' : OpenTopoMap
                       ,'Image' : Esri_WorldImagery
                       };
       var overlayLayers= {
                    /*
                      'Topog': L.tileLayer.provider('OpenSeaMap'),
                      'Topo' : OpenTopoMap,
                      'Image' : Esri_WorldImagery
                    */
                       };

         var layerControl = L.control.layers(
                                      baseLayers
                                      , overlayLayers
                           , {
                               collapsed: true
                              ,position: 'topleft'
                              }).addTo(mymap);
           //scale line with km
             L.control.scale(
                              {
                                imperial: false
                              }).addTo(mymap);

         var sondeControl = L.control({
                              position: 'topright'
                              });

         sondeControl.onAdd = function (map) {

         var div = L.DomUtil.create('div', 'Sondy');

         //var labels = ["http://datentaeter.de/wp-content/uploads/2016/06/flag_de.png","http://datentaeter.de/wp-content/uploads/2016/06/flag_de.png"];

         // loop through our density intervals and generate a label with a colored square for each interval
          return div;
         };
        sondeControl.addTo(mymap);


      var predictControl = L.control({
                              position: 'topright'
                              });

         predictControl.onAdd = function (map) {

         var div = L.DomUtil.create('div', 'Predict');

          return div;
         };



        predictControl.addTo(mymap);



       // document.getElementsByClassName('Sondy leaflet-control')[0].innerHTML;

       // alert(document.getElementsByClassName('Sondy leaflet-control')[0].innerHTML);


function resizeLayerControl() {
  var layerControlHeight = document.body.clientHeight - (10 + 50);
  var layerControl = document.getElementsByClassName('leaflet-control-layers-expanded')[0];
  layerControl.style.overflowY = 'auto';
  layerControl.style.maxHeight = layerControlHeight + 'px';
}
//mymap.on('resize', resizeLayerControl);
//resizeLayerControl();

/*
	L.marker([51.3483, 19.8733],{icon:balon3HighIcon}).addTo(mymap)
		.bindPopup("<b>testMap</b><br />Popup to jest").openPopup();

	L.circle([51.3483, 19.8733], 100, {
		color: 'red',
		fillColor: '#f03',
		fillOpacity: 0.5
	}).addTo(mymap).bindPopup("Zasięg odbioru.");

	L.polygon([
		[51.3463, 19.8723],
		[51.3473, 19.8743],
		[51.3493, 19.8743]
	]).addTo(mymap).bindPopup("Obszar poszukiwań.");
*/

	var popup = L.popup();

	function onMapClick(e) {
		popup
			.setLatLng(e.latlng)
                        .setContent(
                                     "<div><input type=\"text\" value=\""+e.latlng.lat.toFixed(5)+" "+e.latlng.lng.toFixed(5)+"\" id=\"myInput\">"+
                                     "<button onclick=\"clipboardCopyFun('myInput')\">Copy</button></div>"
                                   )
			.openOn(mymap);
	}

//	mymap.on('click', onMapClick);



        function sondeOnMap(sondePar) {
          L.marker([sondePar.latitude, sondePar.longitude]).addTo(mymap)
                .bindPopup(
                           "<div>"
                           +"<b>"+sondePar.Id+"</b>"
                           +"<br>"+getSondeDetailContent(sondePar)
                           +"<input style='display:none' type=\"text\" value=\""+sondePar.latitude+" "+sondePar.longitude+"\" id=\"sondeInfo\">"
                           +"<button onclick=\"clipboardCopyFun('sondeInfo')\">Copy</button>"
                           +"</div>"
                          );

        }
        function sondeOnMapJump(sondeId) {
                  if ( activeSondeId==sondeId ) {
                       activeSondeId = null;
                  } else{
                      activeSondeId=sondeId;
                      sondesParams[sondeId].mapJump(mymap);
                  }
        }

        function predictOnMapJump(predictId) {
                  //alert (predicts[predictId].marker.getLatLng());
                  predicts[predictId].marker.openPopup();
                  mymap.panTo([predicts[predictId].latitude,predicts[predictId].longitude]);
        }


</script>
<script>

var csvData=[];
var menuInner="";
var sondes=[];
//var sondeLayer=null;


function toggleSondeDetail(id,idBtn) {
    var div = document.getElementById(id);
    div.style.display = div.style.display == "none" ? "block" : "none";
    var btn = document.getElementById(idBtn);
    btn.innerHTML=div.style.display == "none" ? "+" : "-";
}

function updateMenuItem(sondePar){
 var idItem="menu_item_"+sondePar.Id;
 var idBtnSondeDetail="sonde_btn_detail_"+sondePar.Id;
 var idSondeDetail="sonde_detail_"+sondePar.Id;
 var div = document.getElementById(idItem);

if (div  === null) {
 item= "<div id=\""+idItem+"\">"
     +"<button "
     +"onclick=\"sondeOnMapJump('"+sondePar.Id+"')\">" +sondePar.Id+"</button>"
     +"<button id=\""+idBtnSondeDetail+"\""
     +"onclick=\"toggleSondeDetail('"+idSondeDetail+"','"+idBtnSondeDetail+"')\" "
     +">"+"+</button>"
     +"<div id=\""+idSondeDetail+"\" style='color:white;display:none'>"
     +sondePar.getSondeDetailContent();
     +"</div>";

var newItem=document.createElement("div");
newItem.id=idItem;
newItem.innerHTML=item;
var menu= document.getElementsByClassName('Sondy leaflet-control')[0];
//menu.insertBefore(newItem,menu.childNodes[0]);
  menu.appendChild(newItem);
}
else
{
var sondeDetail= document.getElementById(idSondeDetail);
//alert(sondeDetail);

sondeDetail.innerHTML=sondePar.getSondeDetailContent();
}

}




function getMapData(){
    nocache = "&nocache="+ Math.random() * 1000000;
    var request = new XMLHttpRequest();
    request.onreadystatechange = function(){
        if (this.readyState == 4){
            if (this.status == 200){
                if (this.responseText != null){
                    //document.getElementById("mapmenu").innerHTML = this.responseText;
                    csvData=this.responseText.split("\n");
                    csvData.splice(-1);
                     menuInner="";

                     //sondeLayer=null;
                     for(line in csvData  ) {
                     if (line>9) break;

                     var sondeParTmp=new SondeParser(csvData[line]);
                       if(typeof sondesParams[sondeParTmp.Id] === 'undefined') {
                        // does not exist
                        //alert("id:"+sondeParTmp.time+" "+csvData[line]);
                         var sonde=new Sonde(sondeParTmp,mymap);
                         sondesParams[sondeParTmp.Id]=sonde;
                       }
                       else {
                             sondesParams[sondeParTmp.Id].copyPosition(sondeParTmp);
                            }
                      updateMenuItem(sondesParams[sondeParTmp.Id]);

                      /*if (sondePar.altitude>0) {
                      var sondemarker=L.marker([sondePar.latitude, sondePar.longitude])
                            .bindPopup("<b>"+sondePar.Id+"</b><br />Popup to jest");
                      sondes.push(sondemarker);
                      sondeOnMap(sondePar);

                       }
*/
                      }
                      /* var overlayMaps = {
                            "Cities": sondes
                       };
                     */
                       // L.control.layers(overlayMaps).addTo(mymap);
                       //sondeLayer=L.LayerGroup(sondes);

                }
            }
        }
    }
    request.open("GET", "getcsv.php", true);
    request.send(null);
}


</script>



</body>
</html>


