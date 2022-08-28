// Declare a button to refresh the control status
let btnRefreshControl = document.getElementById("refreshControls");
refreshControlInfo(); // display at page load

// declare and event on the button to refresh the status
btnRefreshControl.addEventListener('click', refreshControlInfo);

// declare a function to refresh gauge every 10 seconds
setInterval( refreshGaugeData(), 10000);


function setCheckBoxText() {
  var checkBox = document.getElementById("FiltrationPumpCB");
  if (checkBox.checked == true){
    //checkBox.textContent = "ON";
	document.getElementById("FiltrationPumpCB").setAttribute("textContent", "ON");
  } else {
	document.getElementById("FiltrationPumpCB").setAttribute("textContent", "OFF");
  }
}

// --------------------------------------------------------------
// Get data for gauge display
// --------------------------------------------------------------
function refreshGaugeData() {
	var xhttp = new XMLHttpRequest();
	
	xhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			var jsonResponse = JSON.parse(this.responseText);
			document.getElementById("gaugeTempAirIn" ).setAttribute("data-value", jsonResponse.IATemp);
			document.getElementById("gaugeTempAirOut").setAttribute("data-value", jsonResponse.OATemp);
			document.getElementById("gaugeTempWater" ).setAttribute("data-value", jsonResponse.WaTemp);
			document.getElementById("gaugePH"        ).setAttribute("data-value", jsonResponse.pH);
			document.getElementById("gaugeORP"       ).setAttribute("data-value", jsonResponse.Orp);
			document.getElementById("gaugePressure"  ).setAttribute("data-value", jsonResponse.PSI);
			document.getElementById("gaugePhTankFill").setAttribute("data-value", jsonResponse.pHFill);
			document.getElementById("gaugeChlorineTankFill").setAttribute("data-value", jsonResponse.ChFill);
		}
	};

	xhttp.open("GET", "getMeasures", true);
	xhttp.send();
}
// --------------------------------------------------------------
// Get control information and display them in the control table
// --------------------------------------------------------------
function refreshControlInfo() {
	let myTable = document.getElementById('controlTable');
    let controls = [];

	let xhttp = new XMLHttpRequest();
	xhttp.open("GET", "getMeasures", false); // false : means synchronous call
	xhttp.send();
	if (xhttp.readyState == 4 && xhttp.status == 200) {
		let jsonResponse = JSON.parse(xhttp.responseText);
		controls.push({ name:"Auto Mode", value: jsonResponse.Auto});
		controls.push({ name:"Filtration", value: jsonResponse.FPmpS});
		controls.push({ name:"pH- Pump", value: jsonResponse.pHPmpS});
		controls.push({ name:"Chlorine Pump", value: jsonResponse.OrpPmpS});
		controls.push({ name:"pH PID", value: jsonResponse.pHWS});
		controls.push({ name:"Orp PID", value: jsonResponse.ChlWS});
	}

	let headers = ['Control', 'Status'];

    let table = document.createElement('controlTable');
	let headerRow = document.createElement('tr');

    headers.forEach(headerText => {
        let header = document.createElement('th');
        let textNode = document.createTextNode(headerText);
        header.appendChild(textNode);
        headerRow.appendChild(header);
    });
    table.appendChild(headerRow);

    controls.forEach(ctrl => {
        let row = document.createElement('tr');

        Object.values(ctrl).forEach(text => {
            let cell = document.createElement('td');
            let textNode = document.createTextNode(text);
            cell.appendChild(textNode);
            row.appendChild(cell);
        })

        table.appendChild(row);
    });
	if (myTable.hasChildNodes()) {
		let old_table = myTable.firstChild;
		myTable.removeChild(old_table);
	  }
    myTable.appendChild(table);
}