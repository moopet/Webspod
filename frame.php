<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
<?php
// frame.php

include "config.php";

echo "<link rel=\"stylesheet\" href=\"$THEME_PATH/$theme/theme.css\" />\n</head>\n<body class=\"$position\" marginheight=\"0\" topmargin=\"0\">\n";

if ($position == "top")
{
	if ($port > 0)
	{
		if (strlen($world))
			echo "webspod connector | $world | $hostname:$port";
		else
			echo "webspod connector | $hostname:$port";
	}
	else
		echo "webspod connector | not connected";
}
?>
</body>
</html>