<html>
<head>
</head>
<body>
<center>
<object classid="clsid:d27cdb6e-ae6d-11cf-96b8-444553540000" codebase="http://fpdownload.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0" width="<?php echo $_REQUEST['is_popup']==TRUE ? 60 : 300; ?>" height="<?php echo $_REQUEST['is_popup']==TRUE ? 60 : 300; ?>" id="preloader" align="middle">
<param name="allowScriptAccess" value="sameDomain" />
<param name="movie" value="preloader.swf" />
<param name="quality" value="high" />
<param name="bgcolor" value="#ffffff" />
<embed src="../templates/loader/preloader.swf" quality="high" bgcolor="#ffffff" width="<?php echo $_REQUEST['is_popup']==TRUE ? 60 : 300; ?>" height="<?php echo $_REQUEST['is_popup']==TRUE ? 60 : 300; ?>" name="preloader" align="middle" allowScriptAccess="sameDomain" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer" />
</object>
</center>


