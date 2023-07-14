  const timeZonesArray = Object.freeze(
  [
    { "utc" : "(UTC-12:00)", "name" : "Dateline Standard Time", "desc" : "International Date Line West", "offset" : -720},
    { "utc" : "(UTC-11:00)", "name" : "UTC-11", "desc" : "Coordinated Universal Time-11", "offset" : -660},
    { "utc" : "(UTC-10:00)", "name" : "Aleutian Standard Time", "desc" : "Aleutian Islands", "offset" : -600},
    { "utc" : "(UTC-10:00)", "name" : "Hawaiian Standard Time", "desc" : "Hawaii", "offset" : -600},
    { "utc" : "(UTC-09:30)", "name" : "Marquesas Standard Time", "desc" : "Marquesas Islands", "offset" : -570},
    { "utc" : "(UTC-09:00)", "name" : "Alaskan Standard Time", "desc" : "Alaska", "offset" : -540},
    { "utc" : "(UTC-09:00)", "name" : "UTC-09", "desc" : "Coordinated Universal Time-09", "offset" : -540},
    { "utc" : "(UTC-08:00)", "name" : "Pacific Standard Time (Mexico)", "desc" : "Baja California", "offset" : -480},
    { "utc" : "(UTC-08:00)", "name" : "UTC-08", "desc" : "Coordinated Universal Time-08", "offset" : -480},
    { "utc" : "(UTC-08:00)", "name" : "Pacific Standard Time", "desc" : "Pacific Time (US &amp; Canada)", "offset" : -480},
    { "utc" : "(UTC-07:00)", "name" : "US Mountain Standard Time", "desc" : "Arizona", "offset" : -420},
    { "utc" : "(UTC-07:00)", "name" : "Mountain Standard Time (Mexico)", "desc" : "Chihuahua, La Paz, Mazatlan", "offset" : -420},
    { "utc" : "(UTC-07:00)", "name" : "Mountain Standard Time", "desc" : "Mountain Time (US &amp; Canada)", "offset" : -420},
    { "utc" : "(UTC-06:00)", "name" : "Central America Standard Time", "desc" : "Central America", "offset" : -360},
    { "utc" : "(UTC-06:00)", "name" : "Central Standard Time", "desc" : "Central Time (US &amp; Canada)", "offset" : -360},
    { "utc" : "(UTC-06:00)", "name" : "Easter Island Standard Time", "desc" : "Easter Island", "offset" : -360},
    { "utc" : "(UTC-06:00)", "name" : "Central Standard Time (Mexico)", "desc" : "Guadalajara, Mexico City, Monterrey", "offset" : -360},
    { "utc" : "(UTC-06:00)", "name" : "Canada Central Standard Time", "desc" : "Saskatchewan", "offset" : -360},
    { "utc" : "(UTC-05:00)", "name" : "SA Pacific Standard Time", "desc" : "Bogota, Lima, Quito, Rio Branco", "offset" : -300},
    { "utc" : "(UTC-05:00)", "name" : "Eastern Standard Time (Mexico)", "desc" : "Chetumal", "offset" : -300},
    { "utc" : "(UTC-05:00)", "name" : "Eastern Standard Time", "desc" : "Eastern Time (US &amp; Canada)", "offset" : -300},
    { "utc" : "(UTC-05:00)", "name" : "Haiti Standard Time", "desc" : "Haiti", "offset" : -300},
    { "utc" : "(UTC-05:00)", "name" : "Cuba Standard Time", "desc" : "Havana", "offset" : -300},
    { "utc" : "(UTC-05:00)", "name" : "US Eastern Standard Time", "desc" : "Indiana (East)", "offset" : -300},
    { "utc" : "(UTC-05:00)", "name" : "Turks And Caicos Standard Time", "desc" : "Turks and Caicos", "offset" : -300},
    { "utc" : "(UTC-04:00)", "name" : "Paraguay Standard Time", "desc" : "Asuncion", "offset" : -240},
    { "utc" : "(UTC-04:00)", "name" : "Atlantic Standard Time", "desc" : "Atlantic Time (Canada)", "offset" : -240},
    { "utc" : "(UTC-04:00)", "name" : "Venezuela Standard Time", "desc" : "Caracas", "offset" : -240},
    { "utc" : "(UTC-04:00)", "name" : "Central Brazilian Standard Time", "desc" : "Cuiaba", "offset" : -240},
    { "utc" : "(UTC-04:00)", "name" : "SA Western Standard Time", "desc" : "Georgetown, La Paz, Manaus, San Juan", "offset" : -240},
    { "utc" : "(UTC-04:00)", "name" : "Pacific SA Standard Time", "desc" : "Santiago", "offset" : -240},
    { "utc" : "(UTC-03:30)", "name" : "Newfoundland Standard Time", "desc" : "Newfoundland", "offset" : -210},
    { "utc" : "(UTC-03:00)", "name" : "Tocantins Standard Time", "desc" : "Araguaina", "offset" : -180},
    { "utc" : "(UTC-03:00)", "name" : "E. South America Standard Time", "desc" : "Brasilia", "offset" : -180},
    { "utc" : "(UTC-03:00)", "name" : "SA Eastern Standard Time", "desc" : "Cayenne, Fortaleza", "offset" : -180},
    { "utc" : "(UTC-03:00)", "name" : "Argentina Standard Time", "desc" : "City of Buenos Aires", "offset" : -180},
    { "utc" : "(UTC-03:00)", "name" : "Greenland Standard Time", "desc" : "Greenland", "offset" : -180},
    { "utc" : "(UTC-03:00)", "name" : "Montevideo Standard Time", "desc" : "Montevideo", "offset" : -180},
    { "utc" : "(UTC-03:00)", "name" : "Magallanes Standard Time", "desc" : "Punta Arenas", "offset" : -180},
    { "utc" : "(UTC-03:00)", "name" : "Saint Pierre Standard Time", "desc" : "Saint Pierre and Miquelon", "offset" : -180},
    { "utc" : "(UTC-03:00)", "name" : "Bahia Standard Time", "desc" : "Salvador", "offset" : -180},
    { "utc" : "(UTC-02:00)", "name" : "UTC-02", "desc" : "Coordinated Universal Time-02", "offset" : -120},
    { "utc" : "(UTC-01:00)", "name" : "Azores Standard Time", "desc" : "Azores", "offset" : -60},
    { "utc" : "(UTC-01:00)", "name" : "Cape Verde Standard Time", "desc" : "Cabo Verde Is.", "offset" : -60},
    { "utc" : "(UTC+00:00)", "name" : "GMT Standard Time", "desc" : "Dublin, Edinburgh, Lisbon, London", "offset" : 0},
    { "utc" : "(UTC+00:00)", "name" : "Greenwich Standard Time", "desc" : "Monrovia, Reykjavik", "offset" : 0},
    { "utc" : "(UTC+00:00)", "name" : "Sao Tome Standard Time", "desc" : "Sao Tome", "offset" : 0},
    { "utc" : "(UTC+01:00)", "name" : "Morocco Standard Time", "desc" : "Casablanca", "offset" : 60},
    { "utc" : "(UTC+01:00)", "name" : "W. Europe Standard Time", "desc" : "Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna", "offset" : 60},
    { "utc" : "(UTC+01:00)", "name" : "Central Europe Standard Time", "desc" : "Belgrade, Bratislava, Budapest, Ljubljana, Prague", "offset" : 60},
    { "utc" : "(UTC+01:00)", "name" : "Romance Standard Time", "desc" : "Brussels, Copenhagen, Madrid, Paris", "offset" : 60},
    { "utc" : "(UTC+01:00)", "name" : "Central European Standard Time", "desc" : "Sarajevo, Skopje, Warsaw, Zagreb", "offset" : 60},
    { "utc" : "(UTC+01:00)", "name" : "W. Central Africa Standard Time", "desc" : "West Central Africa", "offset" : 60},
    { "utc" : "(UTC+02:00)", "name" : "Jordan Standard Time", "desc" : "Amman", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "GTB Standard Time", "desc" : "Athens, Bucharest", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "Middle East Standard Time", "desc" : "Beirut", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "Egypt Standard Time", "desc" : "Cairo", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "E. Europe Standard Time", "desc" : "Chisinau", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "Syria Standard Time", "desc" : "Damascus", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "West Bank Standard Time", "desc" : "Gaza, Hebron", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "South Africa Standard Time", "desc" : "Harare, Pretoria", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "FLE Standard Time", "desc" : "Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "Israel Standard Time", "desc" : "Jerusalem", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "Kaliningrad Standard Time", "desc" : "Kaliningrad", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "Sudan Standard Time", "desc" : "Khartoum", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "Libya Standard Time", "desc" : "Tripoli", "offset" : 120},
    { "utc" : "(UTC+02:00)", "name" : "Namibia Standard Time", "desc" : "Windhoek", "offset" : 120},
    { "utc" : "(UTC+03:00)", "name" : "Arabic Standard Time", "desc" : "Baghdad", "offset" : 180},
    { "utc" : "(UTC+03:00)", "name" : "Turkey Standard Time", "desc" : "Istanbul", "offset" : 180},
    { "utc" : "(UTC+03:00)", "name" : "Arab Standard Time", "desc" : "Kuwait, Riyadh", "offset" : 180},
    { "utc" : "(UTC+03:00)", "name" : "Belarus Standard Time", "desc" : "Minsk", "offset" : 180},
    { "utc" : "(UTC+03:00)", "name" : "Russian Standard Time", "desc" : "Moscow, St. Petersburg", "offset" : 180},
    { "utc" : "(UTC+03:00)", "name" : "E. Africa Standard Time", "desc" : "Nairobi", "offset" : 180},
    { "utc" : "(UTC+03:30)", "name" : "Iran Standard Time", "desc" : "Tehran", "offset" : 210},
    { "utc" : "(UTC+04:00)", "name" : "Arabian Standard Time", "desc" : "Abu Dhabi, Muscat", "offset" : 240},
    { "utc" : "(UTC+04:00)", "name" : "Astrakhan Standard Time", "desc" : "Astrakhan, Ulyanovsk", "offset" : 240},
    { "utc" : "(UTC+04:00)", "name" : "Azerbaijan Standard Time", "desc" : "Baku", "offset" : 240},
    { "utc" : "(UTC+04:00)", "name" : "Russia Time Zone 3", "desc" : "Izhevsk, Samara", "offset" : 240},
    { "utc" : "(UTC+04:00)", "name" : "Mauritius Standard Time", "desc" : "Port Louis", "offset" : 240},
    { "utc" : "(UTC+04:00)", "name" : "Saratov Standard Time", "desc" : "Saratov", "offset" : 240},
    { "utc" : "(UTC+04:00)", "name" : "Georgian Standard Time", "desc" : "Tbilisi", "offset" : 240},
    { "utc" : "(UTC+04:00)", "name" : "Volgograd Standard Time", "desc" : "Volgograd", "offset" : 240},
    { "utc" : "(UTC+04:00)", "name" : "Caucasus Standard Time", "desc" : "Yerevan", "offset" : 240},
    { "utc" : "(UTC+04:30)", "name" : "Afghanistan Standard Time", "desc" : "Kabul", "offset" : 270},
    { "utc" : "(UTC+05:00)", "name" : "West Asia Standard Time", "desc" : "Ashgabat, Tashkent", "offset" : 300},
    { "utc" : "(UTC+05:00)", "name" : "Ekaterinburg Standard Time", "desc" : "Ekaterinburg", "offset" : 300},
    { "utc" : "(UTC+05:00)", "name" : "Pakistan Standard Time", "desc" : "Islamabad, Karachi", "offset" : 300},
    { "utc" : "(UTC+05:00)", "name" : "Qyzylorda Standard Time", "desc" : "Qyzylorda", "offset" : 300},
    { "utc" : "(UTC+05:30)", "name" : "India Standard Time", "desc" : "Chennai, Kolkata, Mumbai, New Delhi", "offset" : 330},
    { "utc" : "(UTC+05:30)", "name" : "Sri Lanka Standard Time", "desc" : "Sri Jayawardenepura", "offset" : 330},
    { "utc" : "(UTC+05:45)", "name" : "Nepal Standard Time", "desc" : "Kathmandu", "offset" : 345},
    { "utc" : "(UTC+06:00)", "name" : "Central Asia Standard Time", "desc" : "Astana", "offset" : 360},
    { "utc" : "(UTC+06:00)", "name" : "Bangladesh Standard Time", "desc" : "Dhaka", "offset" : 360},
    { "utc" : "(UTC+06:00)", "name" : "Omsk Standard Time", "desc" : "Omsk", "offset" : 360},
    { "utc" : "(UTC+06:30)", "name" : "Myanmar Standard Time", "desc" : "Yangon (Rangoon)", "offset" : 390},
    { "utc" : "(UTC+07:00)", "name" : "SE Asia Standard Time", "desc" : "Bangkok, Hanoi, Jakarta", "offset" : 420},
    { "utc" : "(UTC+07:00)", "name" : "Altai Standard Time", "desc" : "Barnaul, Gorno-Altaysk", "offset" : 420},
    { "utc" : "(UTC+07:00)", "name" : "W. Mongolia Standard Time", "desc" : "Hovd", "offset" : 420},
    { "utc" : "(UTC+07:00)", "name" : "North Asia Standard Time", "desc" : "Krasnoyarsk", "offset" : 420},
    { "utc" : "(UTC+07:00)", "name" : "N. Central Asia Standard Time", "desc" : "Novosibirsk", "offset" : 420},
    { "utc" : "(UTC+07:00)", "name" : "Tomsk Standard Time", "desc" : "Tomsk", "offset" : 420},
    { "utc" : "(UTC+08:00)", "name" : "China Standard Time", "desc" : "Beijing, Chongqing, Hong Kong, Urumqi", "offset" : 480},
    { "utc" : "(UTC+08:00)", "name" : "North Asia East Standard Time", "desc" : "Irkutsk", "offset" : 480},
    { "utc" : "(UTC+08:00)", "name" : "Singapore Standard Time", "desc" : "Kuala Lumpur, Singapore", "offset" : 480},
    { "utc" : "(UTC+08:00)", "name" : "W. Australia Standard Time", "desc" : "Perth", "offset" : 480},
    { "utc" : "(UTC+08:00)", "name" : "Taipei Standard Time", "desc" : "Taipei", "offset" : 480},
    { "utc" : "(UTC+08:00)", "name" : "Ulaanbaatar Standard Time", "desc" : "Ulaanbaatar", "offset" : 480},
    { "utc" : "(UTC+08:45)", "name" : "Aus Central W. Standard Time", "desc" : "Eucla", "offset" : 525},
    { "utc" : "(UTC+09:00)", "name" : "Transbaikal Standard Time", "desc" : "Chita", "offset" : 540},
    { "utc" : "(UTC+09:00)", "name" : "Tokyo Standard Time", "desc" : "Osaka, Sapporo, Tokyo", "offset" : 540},
    { "utc" : "(UTC+09:00)", "name" : "North Korea Standard Time", "desc" : "Pyongyang", "offset" : 540},
    { "utc" : "(UTC+09:00)", "name" : "Korea Standard Time", "desc" : "Seoul", "offset" : 540},
    { "utc" : "(UTC+09:00)", "name" : "Yakutsk Standard Time", "desc" : "Yakutsk", "offset" : 540},
    { "utc" : "(UTC+09:30)", "name" : "Cen. Australia Standard Time", "desc" : "Adelaide", "offset" : 570},
    { "utc" : "(UTC+09:30)", "name" : "AUS Central Standard Time", "desc" : "Darwin", "offset" : 570},
    { "utc" : "(UTC+10:00)", "name" : "E. Australia Standard Time", "desc" : "Brisbane", "offset" : 600},
    { "utc" : "(UTC+10:00)", "name" : "AUS Eastern Standard Time", "desc" : "Canberra, Melbourne, Sydney", "offset" : 600},
    { "utc" : "(UTC+10:00)", "name" : "West Pacific Standard Time", "desc" : "Guam, Port Moresby", "offset" : 600},
    { "utc" : "(UTC+10:00)", "name" : "Tasmania Standard Time", "desc" : "Hobart", "offset" : 600},
    { "utc" : "(UTC+10:00)", "name" : "Vladivostok Standard Time", "desc" : "Vladivostok", "offset" : 600},
    { "utc" : "(UTC+10:30)", "name" : "Lord Howe Standard Time", "desc" : "Lord Howe Island", "offset" : 630},
    { "utc" : "(UTC+11:00)", "name" : "Bougainville Standard Time", "desc" : "Bougainville Island", "offset" : 660},
    { "utc" : "(UTC+11:00)", "name" : "Russia Time Zone 10", "desc" : "Chokurdakh", "offset" : 660},
    { "utc" : "(UTC+11:00)", "name" : "Magadan Standard Time", "desc" : "Magadan", "offset" : 660},
    { "utc" : "(UTC+11:00)", "name" : "Norfolk Standard Time", "desc" : "Norfolk Island", "offset" : 660},
    { "utc" : "(UTC+11:00)", "name" : "Sakhalin Standard Time", "desc" : "Sakhalin", "offset" : 660},
    { "utc" : "(UTC+11:00)", "name" : "Central Pacific Standard Time", "desc" : "Solomon Is., New Caledonia", "offset" : 660},
    { "utc" : "(UTC+12:00)", "name" : "Russia Time Zone 11", "desc" : "Anadyr, Petropavlovsk-Kamchatsky", "offset" : 720},
    { "utc" : "(UTC+12:00)", "name" : "New Zealand Standard Time", "desc" : "Auckland, Wellington", "offset" : 720},
    { "utc" : "(UTC+12:00)", "name" : "UTC+12", "desc" : "Coordinated Universal Time+12", "offset" : 720},
    { "utc" : "(UTC+12:00)", "name" : "Fiji Standard Time", "desc" : "Fiji", "offset" : 720},
    { "utc" : "(UTC+12:45)", "name" : "Chatham Islands Standard Time", "desc" : "Chatham Islands", "offset" : 765},
    { "utc" : "(UTC+13:00)", "name" : "UTC+13", "desc" : "Coordinated Universal Time+13", "offset" : 780},
    { "utc" : "(UTC+13:00)", "name" : "Tonga Standard Time", "desc" : "Nuku&#8217;alofa", "offset" : 780},
    { "utc" : "(UTC+13:00)", "name" : "Samoa Standard Time", "desc" : "Samoa", "offset" : 780},
    { "utc" : "(UTC+14:00)", "name" : "Line Islands Standard Time", "desc" : "Kiritimati Island", "offset" : 840}
  ]);

  const commandEnum = Object.freeze({
    "RGB_LEDS_CHANGE" : 101, // real time
    "TUBES_BRIG"      : 201, // real time
    "CATH_POI"        : 202, // real time
    "WIFI_UPD"        : 301,
    "TIME_UPD"        : 401, // real time
    "DST_UPD"         : 501, // real time
    "NTP_SYNC"        : 601, // real time
    "TIME_SYNC"       : 602  // real time
  });
  
  const modalMsgType = Object.freeze({
    "INFO"    : 101,
    "WARNING" : 102,
    "ERROR"   : 103
  });

$(function () {
  
  /* Show-Hide password button event listener */
  $("#btn-pwd-show").click(function() {
    if ($("#net-key").attr("type") == "password") {
      $("#net-key").prop("type", "text");
      
      $("#icon-pwd-show").addClass("d-none");
      $("#icon-pwd-hide").removeClass("d-none");
    }
    else {
      $("#net-key").prop("type", "password");
      
      $("#icon-pwd-show").removeClass("d-none");
      $("#icon-pwd-hide").addClass("d-none");
    }
  });
  
  /* Sync ranges and text inputs by event listeners */
  setSlider("#range-bri-rgb", "#text-bri-rgb", 0, 100, 1, true);
  setSlider("#range-bri-comm", "#text-bri-comm", 0, 100, 1);
  for (i=1; i<=5; i++) {
    var textName = "#text-bri-tube" + i;
    var rangeName = "#range-bri-tube" + i;
    setSlider(rangeName, textName, 0, 100, 1);
  }
 
  /* Activate color picker plugin */
  $("#rgb-picker").colorpicker({
    format: "rgb",
    useAlpha: false,
    color: "rgb(0,0,0)"
  });
  
  /* Fill time zones field */
  var $dropdown = $("#sel-time-zone");
  timeZonesArray.forEach(function(entry, i) {
    $dropdown.append("<option value=\"" + i + "\">" + entry.utc + " " + entry.name + " - " + entry.desc + "</option>");
  });
  
  /* Edit buttons event listeners */
  $("#edit-wifi, #edit-time, #edit-dst").click(function() {
    var suff = this.id.split("-")[1];
    $("#edit-" + suff).addClass("d-none");
    $("#apply-" + suff).removeClass("d-none");
    $("#cancel-" + suff).removeClass("d-none");
    
    if (suff=="time") {
      $("#time-ntp-sync").addClass("d-none");
      $("#sync-man").addClass("d-none");
      $("#fs-time-1").prop('disabled', false);
      $("#fs-time-2").prop('disabled', false);
      $("#fs-time-ntp").prop('disabled', false);
    }
    else {
      $("#fs-" + suff).prop('disabled', false);
    }
  });
  
  /* Cancel buttons event listeners */
  $("#cancel-wifi, #cancel-time, #cancel-dst").click(function() { cancelApplyBtn(this.id); });
  
  /* Apply wifi event listeners */
  $("#apply-wifi").click(function() {
    var showFooteer = true;
    var showStatic = true;
    var yesNoSenderID = "wifi";
    
    showModalMsg(modalMsgType.WARNING, "The clock will restart and this page may no longer be reachable. Continue?", showFooteer, showStatic, yesNoSenderID);
  });
  
  /* Listeners for wifi ap/client mode radio field */
  $('input[type=radio][name=net-mode]').change(function() {
    if (this.value == 'ap') {
      $("#net-ap-row").removeClass("d-none");
      $("#net-client-row").addClass("d-none");
    }
    else if (this.value == 'client') {
      $("#net-ap-row").addClass("d-none");
      $("#net-client-row").removeClass("d-none");
    }
  });  
  
  /* Other Apply event listeners */
  $("#apply-time").click(function() { constructTimeCommand(); });
  $("#apply-dst").click(function()  { constructDSTCommand(); });
  
  // Force sync NTP button event listener
  $("#ntp-sync-now").click(function() {
    var jsonCmd = {};
    jsonCmd.cmd = commandEnum.NTP_SYNC;
    jsonCmd.data = commandEnum.NTP_SYNC;

    sendCommand(jsonCmd);
  });
  
  /* Modal Yes/No event listener */
  $("#modal-msg-no, #modal-msg-yes").click(function() {
    var sender = $(this).data("sender");
    var action = $(this).data("action");
    
    if ((sender == "wifi") && ( action == "yes")) {
      var showFooteer = false;
      var showStatic = true;
      showModalMsg(modalMsgType.INFO, "Updating the Wi-Fi settings. Close this page and reopen it at the new IP address of the clock", showFooteer, showStatic);
      constructWifiCommand();
    }
    else if ((sender == "wifi") && ( action == "no")) cancelApplyBtn("cancel-wifi");
    
  });
  
  // Manual sync button event listener
  $("#man-sync-now").click(function() { constructTimeSyncCmd(); });

  /* Listeners for use/not-use ntp radio field */
  $('input[type=radio][name=time-sync]').change(function() {
    if (this.value == 'ntp')      $("#sync-ntp").removeClass("d-none");
    else if (this.value == 'man') $("#sync-ntp").addClass("d-none");
  });  
  
  /* Load json settings files and fill the page */
  fillSettingsFields(["version", "wifi", "time", "dst", "rgb", "tubes"]);
  
  /* Direct switches events listeners */
  $("#switch-rgb-on").change(function()      { constructRgbLedsCommand(); });
  $("#switch-cathode-poi").change(function() {
    $("#fs-cathode-poi-h").prop('disabled', !$("#switch-cathode-poi").is(":checked"));
    constructCatodePoiCommand();
  });
  
  // Number field listener (without associated slider)
  $("#cathode-poi-h").on("change", function(){ constructCatodePoiCommand(); });
  $("#cathode-poi-min").on("change", function(){ constructCatodePoiCommand(); });
  $("#cathode-poi-duration").on("change", function(){ constructCatodePoiCommand(); });
  
  /* Tubes brigthness switch event listener */
  $("#switch-same-bri").change(function() {
    toggleBriInputs(true);
    constructTubesBriCmd();
  });
  
  /* Color picker event listeners */
  $("#rgb-picker").colorpicker().on("change", function () {
    if ( $("#rgb-picker").hasClass("first-load") ) {
      $("#rgb-picker").removeClass("first-load");
    }
    else {
      constructRgbLedsCommand();
    }
  });
  
});

function showModalMsg(modalType, message, showFooteer, showStatic, yesNoSenderID="") {
  var title;
  var titleBg;
  
  if (modalType == modalMsgType.INFO) {
    title = "Info";
    titleBg = "bg-primary";
  }
  else if (modalType == modalMsgType.WARNING) {
    title = "Warning";
    titleBg = "bg-warning";
  }
  else if (modalType == modalMsgType.ERROR) {
    title = "Error";
    titleBg = "bg-danger";
  }
  else return;
  
  if (yesNoSenderID == "") {
    $("#modal-msg-no, #modal-msg-yes").addClass("d-none");
    $("#modal-msg-close").removeClass("d-none");
  }
  else {
    $("#modal-msg-no, #modal-msg-yes").data("sender", yesNoSenderID);
    
    $("#modal-msg-no, #modal-msg-yes").removeClass("d-none");
    $("#modal-msg-close").addClass("d-none");
  }
  
  if (showFooteer)
    $("#modal-msg-footer, #modal-msg-x").removeClass("d-none");
  else
    $("#modal-msg-footer, #modal-msg-x").addClass("d-none");
  
  $("#modal-msg-header").removeClass("bg-primary bg-warning bg-danger");
  $("#modal-msg-header").addClass(titleBg);
  
  $("#modal-msg-title").text(title);
  $("#modal-msg-message").text(message);
  
  if (showStatic)
    $('#modal-msg-box').modal({backdrop: 'static', keyboard: false});
  else
    $('#modal-msg-box').modal("show");
}

function hideModalMsg() {
  $('#modal-msg-box').modal("hide");
}

function cancelApplyBtn(sender) {
  var suff = sender.split("-")[1];
  $("#edit-" + suff).removeClass("d-none");
  $("#apply-" + suff).addClass("d-none");
  $("#cancel-" + suff).addClass("d-none");

  if (suff == "time") {
    $("#fs-time-1").prop('disabled', true);
    $("#fs-time-2").prop('disabled', true);
    $("#fs-time-ntp").prop('disabled', true);
  }
  else {
    $("#fs-" + suff).prop('disabled', true);
  }
  
  fillSettingsFields([suff]);
}

function setSlider(sliderName, textName, min, max, step, rgb = false) {
  $(sliderName).attr({
    "max"  : max,
    "min"  : min,
    "step" : step
  });
  
  $(textName).attr({
    "max"  : max,
    "min"  : min,
    "step" : step
  });
  
  $(textName).on("input", function(){
    $(sliderName).val($(this).val());
  });
  
  $(sliderName).on("input", function(){
    $(textName).val($(this).val());
  });
  
  $(textName).on("change", function(){
    if (!rgb)
      constructTubesBriCmd();
    else
      constructRgbLedsCommand();
  });
  
  $(sliderName).on("change", function(){
    if (!rgb)
      constructTubesBriCmd();
    else
      constructRgbLedsCommand();
  });
}

function constructTimeSyncCmd() {
  var value = $("#man-time").val();  
  var matches = value.match(/^(\d{1,2})\/(\d{1,2})\/(\d{4})\s(\d{1,2}):(\d{1,2}):(\d{1,2})$/);

  if (matches === null) return false;
  else {
    // check the date sanity
    var year = parseInt(matches[3], 10);
    var month = parseInt(matches[2], 10) - 1; // months are 0-11
    var day = parseInt(matches[1], 10);
    var hour = parseInt(matches[4], 10);
    var minute = parseInt(matches[5], 10);
    var second = parseInt(matches[6], 10);
    var date = new Date(year, month, day, hour, minute, second);
    if ( date.getFullYear() !== year
      || date.getMonth() != month
      || date.getDate() !== day
      || date.getHours() !== hour
      || date.getMinutes() !== minute
      || date.getSeconds() !== second ) return false;
    
    var jsonCmd = {};
    
    jsonCmd.cmd = commandEnum.TIME_SYNC;
    
    jsonCmd.h = hour;
    jsonCmd.m = minute;
    jsonCmd.s = second;
    
    jsonCmd.d  = day;
    jsonCmd.mt = month + 1;
    jsonCmd.y  = year;
    
    sendCommand(jsonCmd);
    return true;
  }
}

function constructCatodePoiCommand() {
  var jsonCmd = {};
  jsonCmd.cmd = commandEnum.CATH_POI;
  
  jsonCmd.cath_p_on  = $("#switch-cathode-poi").is(":checked");
  jsonCmd.cath_p_h   = parseInt($("#cathode-poi-h").val());
  jsonCmd.cath_p_min = parseInt($("#cathode-poi-min").val());
  jsonCmd.cath_p_dur = parseInt($("#cathode-poi-duration").val());
  
  sendCommand(jsonCmd);
}

function constructTubesBriCmd() {
  var jsonCmd = {};
    
  jsonCmd.cmd = commandEnum.TUBES_BRIG;
  jsonCmd.tube_bri = [];
  
  if ( $("#switch-same-bri").is(":checked") ) {
    for (i=1; i<=5; i++)
      jsonCmd.tube_bri.push( parseInt($("#range-bri-comm").val()) );
  }
  else {
    for (i=1; i<=5; i++)
      jsonCmd.tube_bri.push( parseInt($("#range-bri-tube" + (i)).val()) );
  }
  
  sendCommand(jsonCmd);
}

function constructRgbLedsCommand() {
  var jsonCmd = {};
  
  jsonCmd.cmd = commandEnum.RGB_LEDS_CHANGE;
  
  jsonCmd.rgb_on  = $("#switch-rgb-on").is(":checked");;
  jsonCmd.rgb_bri = parseInt($("#range-bri-rgb").val());
  
  var rgbColor = $("#rgb-picker").colorpicker("color").api("rgb");
  
  var red   = Math.round(rgbColor._color.color[0]) << 16;
  var green = Math.round(rgbColor._color.color[1]) << 8;
  var blue  = Math.round(rgbColor._color.color[2]);
  
  jsonCmd.rgb_color = red + green + blue;
  
  sendCommand(jsonCmd);
}

function constructWifiCommand() {
  var jsonCmd = {};
  
  jsonCmd.cmd = commandEnum.WIFI_UPD;
  
  jsonCmd.ssid = $("#net-ssid").val();
  jsonCmd.key = $("#net-key").val();
  jsonCmd.ap_mode = $("#net-mode-ap").is(":checked");
  
  sendCommand(jsonCmd);
}

function constructTimeCommand() {
  var jsonCmd = {};
  
  jsonCmd.cmd = commandEnum.TIME_UPD;
  
  jsonCmd.format_12_h = $("#switch-time-format").is(":checked");
  jsonCmd.ntp_sync = $("#time-sync-ntp").is(":checked");
  jsonCmd.ntp_addr = $("#ntp-addr").val();
  jsonCmd.ntp_sync_int_h = parseInt($("#ntp-interval").val());
  jsonCmd.time_zone_enable = $("#switch-auto-timezone").is(":checked");
  
  var zoneId = parseInt($('select[name=sel-time-zone] option').filter(':selected').val());
  jsonCmd.time_zone_id = zoneId;
  
  jsonCmd.time_zone_offset = timeZonesArray[zoneId].offset;  
  
  sendCommand(jsonCmd);
}

function constructDSTCommand() {
  var jsonCmd = {};
  
  jsonCmd.cmd = commandEnum.DST_UPD;
  
  jsonCmd.dst_enable = $("#switch-auto-dst").is(":checked");
  jsonCmd.dst_offset = parseInt($("#dst-offset").val());
  
  jsonCmd.dst_start = {}
  
  jsonCmd.dst_start.week = parseInt($('select[name=sel-dst-start-w] option').filter(':selected').val());
  jsonCmd.dst_start.week_day = parseInt($('select[name=sel-dst-start-d] option').filter(':selected').val());
  jsonCmd.dst_start.month = parseInt($('select[name=sel-dst-start-m] option').filter(':selected').val());
  jsonCmd.dst_start.h = parseInt($("#dst-start-h").val());
  
  jsonCmd.dst_end = {}
  
  jsonCmd.dst_end.week = parseInt($('select[name=sel-dst-end-w] option').filter(':selected').val());
  jsonCmd.dst_end.week_day = parseInt($('select[name=sel-dst-end-d] option').filter(':selected').val());
  jsonCmd.dst_end.month = parseInt($('select[name=sel-dst-end-m] option').filter(':selected').val());
  jsonCmd.dst_end.h = parseInt($("#dst-end-h").val());
  
  sendCommand(jsonCmd);
}

function riseLoadingError() {
  var showFooteer = false;
  var showStatic = true;
  showModalMsg(modalMsgType.ERROR, "Loading failed. Please reload the page", showFooteer, showStatic);
}

/* Page filler function */
function fillSettingsFields(groupArr) {
  fillVersion = ( jQuery.inArray("version", groupArr)  > -1 );
  fillWifi    = ( jQuery.inArray("wifi", groupArr)  > -1 );
  fillTime    = ( jQuery.inArray("time", groupArr)  > -1 );
  fillDST     = ( jQuery.inArray("dst", groupArr)   > -1 );
  fillRGB     = ( jQuery.inArray("rgb", groupArr)   > -1 );
  fillTubes   = ( jQuery.inArray("tubes", groupArr) > -1 );
  
  
  // fill version section
  if (fillVersion) {
    $.ajax({
      type: "GET",
      url: "/json/version.json",
      dataType: "json",
      success: function (data) {
        
        $("#firmware-v").text(data.firmwareV);
        $("#web-v").text(data.dataV);
      },
      error: function( jqXhr, textStatus, errorThrown ) { riseLoadingError(); }
    });
  }
  
  
  // fill wifi section
  if (fillWifi) {
    $.ajax({
      type: "GET",
      url: "/json/net.json",
      dataType: "json",
      success: function (data) {
        
        $("#net-ssid-ap").val( data.ap_default_ssid );
        $("#net-ssid").val( data.ssid );
        $("#net-key").val( data.key );

        $("#net-mode-ap").prop("checked", data.ap_mode);
        $("#net-mode-client").prop("checked", !data.ap_mode);
        
        if (data.ap_mode) {
          $("#net-ap-row").removeClass("d-none");
          $("#net-client-row").addClass("d-none");
        }
        else {
          $("#net-ap-row").addClass("d-none");
          $("#net-client-row").removeClass("d-none");
        }
      },
      error: function( jqXhr, textStatus, errorThrown ) { riseLoadingError(); }
    });
  }
  
  // fill actual time, date and last sync
  if (fillTime) {
    $.ajax({
      type: "GET",
      url: "/json/time.json",
      dataType: "json",
      success: function (data) {

        if (data.last_sync.y < 0) $("#ntp-last-sync").val("unknown");
        else {
          $("#ntp-last-sync").val(twoDig(data.last_sync.d) + "/" + 
                                  twoDig(data.last_sync.mt) + "/" + 
                                  data.last_sync.y + " " + 
                                  twoDig(data.last_sync.h) + ":" + 
                                  twoDig(data.last_sync.m) + ":" + 
                                  twoDig(data.last_sync.s) );
        }

        $("#man-time").val( twoDig(data.now.d) + "/" + twoDig(data.now.mt) + "/" + data.now.y + " " +
                            twoDig(data.now.h) + ":" + twoDig(data.now.m) + ":" + twoDig(data.now.s) );
      },
      error: function( jqXhr, textStatus, errorThrown ) { riseLoadingError(); }
    });
  }

  // fill other sections
  if (fillTime || fillDST || fillRGB || fillTubes) {
    $.ajax({
      type: "GET",
      url: "/json/general.json",
      dataType: "json",
      success: function (data) {

        // fill time section
        if (fillTime) {
          $("#switch-time-format").prop( "checked", data.format_12_h );
          
          $("#time-sync-ntp").prop("checked", data.ntp_sync);
          $("#time-sync-man").prop("checked", !data.ntp_sync);
          
          if (data.ntp_sync) {
            $("#sync-ntp").removeClass("d-none");
            $("#time-ntp-sync").removeClass("d-none");
            $("#sync-man").addClass("d-none");
          }
          else {
            $("#sync-ntp").addClass("d-none");
            $("#sync-man").removeClass("d-none");
          }
          
          $("#ntp-addr").val( data.ntp_addr );
          $("#ntp-interval").val( data.ntp_sync_int_h );
          $("#switch-auto-timezone").prop( "checked", data.time_zone_enable );
          $("#sel-time-zone option[value=\"" + data.time_zone_id + "\"]").prop('selected', true);
        }
        
        // fill DST section
        if (fillDST) {
          $("#switch-auto-dst").prop( "checked", data.dst_enable );
          $("#dst-offset").val( data.dst_offset );
          
          $("#sel-dst-start-w option[value=\"" + data.dst_start.week + "\"]").prop('selected', true);
          $("#sel-dst-start-d option[value=\"" + data.dst_start.week_day + "\"]").prop('selected', true);
          $("#sel-dst-start-m option[value=\"" + data.dst_start.month + "\"]").prop('selected', true);
          $("#dst-start-h").val( data.dst_start.h );
          
          $("#sel-dst-end-w option[value=\"" + data.dst_end.week + "\"]").prop('selected', true);
          $("#sel-dst-end-d option[value=\"" + data.dst_end.week_day + "\"]").prop('selected', true);
          $("#sel-dst-end-m option[value=\"" + data.dst_end.month + "\"]").prop('selected', true);
          $("#dst-end-h").val( data.dst_end.h );
        }
        
        
        // fill tubes section
        if (fillTubes) {
          var sameBri = true;
          for (i=0; i<5; i++) {
            $("#text-bri-tube" + (i+1)).val( data.tube_bri[i] );
            $("#range-bri-tube" + (i+1)).val( data.tube_bri[i] );
            if (data.tube_bri[i] != data.tube_bri[0])
              sameBri = false;
          }
          $("#switch-same-bri").prop( "checked", sameBri );
          toggleBriInputs(false);
          $("#switch-cathode-poi").prop( "checked", data.cath_p_on );
          $("#cathode-poi-h").val( data.cath_p_h );
          $("#cathode-poi-min").val( data.cath_p_min );
          $("#cathode-poi-duration").val( data.cath_p_dur );
          $("#fs-cathode-poi-h").prop('disabled', !data.cath_p_on);
        }

        // fill rgb leds section
        if (fillRGB) {
          $("#switch-rgb-on").prop( "checked", data.rgb_on );
          $("#text-bri-rgb").val( data.rgb_bri );
          $("#range-bri-rgb").val( data.rgb_bri );

          $("#rgb-picker").colorpicker("setValue", "rgb(" + data.rgb_color[0] + ", " + data.rgb_color[1] + ", " + data.rgb_color[2] + ")");
        }
      },
      error: function( jqXhr, textStatus, errorThrown ) { riseLoadingError(); }
    });
  }
  
}

function toggleBriInputs(updateVal) {
  if ( $("#switch-same-bri").is(":checked") ) {
    $("#tubes-common-bri").removeClass("d-none");
    $("#tubes-single-bri").addClass("d-none");
    
    var meanVal = 0;
    for (i=1; i<=5; i++)
      meanVal += parseInt($("#range-bri-tube" + (i)).val(), 10);
      
    meanVal = meanVal/5;
    meanVal = Math.round(meanVal);
    
    $("#text-bri-comm").val( meanVal );
    $("#range-bri-comm").val( meanVal );
  }
  else {
    $("#tubes-common-bri").addClass("d-none");
    $("#tubes-single-bri").removeClass("d-none");
    
    if (updateVal) { // false for page loading
      var commValue = parseInt($("#range-bri-comm").val(), 10);
      
      for (i=0; i<5; i++) {
        $("#text-bri-tube" + (i+1)).val( commValue );
        $("#range-bri-tube" + (i+1)).val( commValue );
      }
    }
  }
}

function sendCommand(jsonCmd) {
  console.log(jsonCmd);
  
  $.ajax({
      url: 'update',
      dataType: 'json',
      type: 'post',
      contentType: 'application/json',
      data: JSON.stringify( jsonCmd ),
      processData: false,
      success: function( data ){
        console.log("Send command success");
        
        if      (jsonCmd.cmd == commandEnum.WIFI_UPD) cancelApplyBtn("cancel-wifi");
        else if ((jsonCmd.cmd == commandEnum.TIME_UPD) || (jsonCmd.cmd == commandEnum.NTP_SYNC)) cancelApplyBtn("cancel-time");
        else if (jsonCmd.cmd == commandEnum.DST_UPD)  cancelApplyBtn("cancel-dst");
      },
      error: function( jqXhr, textStatus, errorThrown ){
        if ( jqXhr.status == 500) {
          msg = jqXhr.responseText; 
        }
        else {
          msg = "Update failed"
        }
        var showFooteer = true;
        var showStatic = false;
        showModalMsg(modalMsgType.ERROR, msg, showFooteer, showStatic);
        console.log( msg );
      }
  });
}

function twoDig(number) {
  return ('0' + number).slice(-2);
}