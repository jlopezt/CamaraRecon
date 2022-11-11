function aCasa() {
    var x = location.href;
    
    var cadenaPartida = x.split("/");
    var salida="";
    
    for (var i=0; i < cadenaPartida.length-1; i++) console.log("trozo["+i+"]:"+cadenaPartida[i]);

    for (var i=0; i < cadenaPartida.length-1; i++) {
      if(salida!="") salida +="/";
      salida+=cadenaPartida[i];
      }
    salida += "/";  
    window.location.href = salida;
  }

document.addEventListener("DOMContentLoaded", function(event) {
  var baseHost = document.location.origin;
  var streamUrl = baseHost + ":81";
  //const WS_URL = "ws://" + window.location.host + ":88";
  const WS_URL = "wss://jlopezt:88716@domoticae.lopeztola.com/camaraws/";
  var ws;// = new WebSocket(WS_URL);

  const home = document.getElementById("button-home"); //boton de home
  const view = document.getElementById("stream"); //la imagen
  const viewfoto = document.getElementById("foto"); //Imagen para foto
  const personFormField = document.getElementById("person"); //Cuadro de texto para escribir el nombre
  const streamButton = document.getElementById("button-stream"); //Boton Visualizar
  const captureButton = document.getElementById("button-capture"); //Boton capturar
  const deleteAllButton = document.getElementById("delete_all"); //Boton borrar todas
  const serializeButton=document.getElementById("button-serialize"); //Boton Serializar
  const fotoButton=document.getElementById("button-foto"); //Boton foto
  const reconocerCheck=document.getElementById("check-recon"); //check para activar/desactivar el reconocimiento

  var flagFoto=false;

  var myVar =setInterval(reconnect, 5000);

  function reconnect() {
    console.log("Evaluando reconexion...");  

    /*readyState
    0	CONNECTING	Socket has been created. The connection is not yet open.
    1	OPEN	The connection is open and ready to communicate.
    2	CLOSING	The connection is in the process of closing.
    3	CLOSED	The connection is closed or couldn't be opened.
    */  
    if(ws.readyState<2) return;
    console.log(`Reconectando a  ${WS_URL}`);
    connect();
  }

  function connect() {
    ws = new WebSocket(WS_URL);

    ws.onopen = () => 
      {
      console.log(`Conectando a ${WS_URL}`);
      ws.send("stream");
      };
    
    ws.onclose = function(e) 
      {
      console.log('Socket cerrado.', e.reason);
      };

    ws.onerror = function(err) 
      {
      console.error('Socket error: ', err.message, 'Cerrando el socket');
      ws.close();
      };
    
    ws.onmessage = message => 
      {
      if (typeof message.data === "string") 
        {
        console.log('se ha recibido: %s',message.data);
        if (message.data.substr(0, 8) == "listface") {addFaceToScreen(message.data.substr(9));} 
        else if (message.data == "delete_faces") {deleteAllFacesFromScreen();} 
        else if (message.data.substr(0, 10) == "depurador:") {document.getElementById("depurador").innerHTML = message.data;} 
        else {document.getElementById("current-status").innerHTML = message.data;}
        }
      if (message.data instanceof Blob) 
        {
        console.log('se han recibido datos binarios');
        if(flagFoto==true)
          {       
          console.log('¡¡¡Es una foto!!!');
          flagFoto=false;  
          var urlObject = URL.createObjectURL(message.data);
          viewfoto.src = urlObject;
          }
        else
          {
          var urlObject = URL.createObjectURL(message.data);
          view.src = urlObject;
          }
        }
      }
  };

  reconocerCheck.onchange = () => 
    {
    if(reconocerCheck.checked) 
      {
      ws.send("reconoce");
      console.log('Reconocimiento on');   
      }
    else
      {
      ws.send("no_reconoce");
      console.log('Reconocimiento off');   
      }
    }

  streamButton.onclick = () => 
    {
    ws.send("stream");
    flagFoto=false;  
    };

  serializeButton.onclick = () => 
    {
    ws.send("serialize");
    };

  fotoButton.onclick = () => 
    {
    ws.send("foto");
    flagFoto=true;
    };

  captureButton.onclick = () => 
    {
    person_name = document.getElementById("person").value;
    ws.send("capture:" + person_name);
    };

  deleteAllButton.onclick = () => 
    {
    ws.send("delete_all");
  };
  
  personFormField.onkeyup = () => 
    {
    captureButton.disabled = false;
    };

  function deleteAllFacesFromScreen() 
    {
    // deletes face list in browser only
    const faceList = document.querySelector("ul");
    while (faceList.firstChild) 
      {
      faceList.firstChild.remove();
      }
    personFormField.value = "";
    captureButton.disabled = true;
    }

  function addFaceToScreen(person_name) 
    {
    const faceList = document.querySelector("ul");
    let listItem = document.createElement("li");
    let closeItem = document.createElement("span");
    closeItem.classList.add("delete");
    closeItem.id = person_name;
    closeItem.addEventListener("click", function() 
      {
      ws.send("remove:" + person_name);
      });
    listItem.appendChild(document.createElement("strong")).textContent = person_name;
    listItem.appendChild(closeItem).textContent = "X";
    faceList.appendChild(listItem);
    }

  captureButton.disabled = true;
  connect();
  });    