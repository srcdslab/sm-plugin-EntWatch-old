<?php
include_once 'init.php';
require_once('protect.php');
require_once('steamid.php');
require_once('connect.php');

include_once(INCLUDES_PATH . "/callbacks.php");
$xajax->processRequests();
session_start();
print_r($xajax->printJavascript("", "xajax.js"));

function declension($digit,$expr,$onlyword=false)
{
	    if(!is_array($expr)) $expr = array_filter(explode(' ', $expr));
	    if(empty($expr[2])) $expr[2]=$expr[1];
	    $i=preg_replace('/[^0-9]+/s','',$digit)%100;
	    if($onlyword) $digit='';
	    if($i>=5 && $i<=20) $res=$digit.' '.$expr[2];
	    else
	    {
	        $i%=10;
	        if($i==1) $res=$digit.' '.$expr[0];
	        elseif($i>=2 && $i<=4) $res=$digit.' '.$expr[1];
	        else $res=$digit.' '.$expr[2];
	    }
	    return trim($res);
}

function duration_string($input_duration)
{
	if($input_duration<0)
	{
		$return_duration = "Invalid Time";
		return $return_duration;
	} elseif($input_duration==0)
	{
		$return_duration = "Permanent";
		return $return_duration;
	} else
	{
		$count_week = floor($input_duration/10080);
		$count_days = floor(($input_duration%10080)/1440);
		$count_hours = floor(($input_duration%1440)/60);
		$count_minutes = floor($input_duration%60);
		$return_duration='';
		if($count_week>0) $return_duration.= declension($count_week,array('<span key_phrase="Week_1" class="lang">Week</span>','<span key_phrase="Week_2" class="lang">Weeks</span>','<span key_phrase="Week_3" class="lang">Weeks</span>')).' ';
		if($count_days>0) $return_duration.= declension($count_days,array('<span key_phrase="Day_1" class="lang">Day</span>','<span key_phrase="Day_2" class="lang">Days</span>','<span key_phrase="Day_3" class="lang">Days</span>')).' ';
		if($count_hours>0) $return_duration.= declension($count_hours,array('<span key_phrase="Hour_1" class="lang">Hour</span>','<span key_phrase="Hour_2" class="lang">Hours</span>','<span key_phrase="Hour_3" class="lang">Hours</span>')).' ';
		if($count_minutes>0) $return_duration.= declension($count_minutes,array('<span key_phrase="Minute_1" class="lang">Minute</span>','<span key_phrase="Minute_2" class="lang">Minutes</span>','<span key_phrase="Minute_3" class="lang">Minutes</span>')).' ';
		return $return_duration;
	}
}
?>

<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
		<title>EntWatch Bans</title>
		<script src="jquery.3.6.0.js"></script>
		<script src="lang.js"></script>
		<script src="ebans.js"></script>
		<script src="xajax.js"></script>
		<link id="theme" rel="stylesheet" href="css/themes/black/main.css">
	</head>
	<body onload="var lang = localStorage.getItem('lang') || 'en'; SetLang(lang);">
		<div class="desktop">
			<nav class="navbar">
				<div class="navbar-container">
					<div class="navbar-left">
						<a key_phrase="EBan List Link" class="lang" href="./?page=1">EBan List</a>
						<?php
							if ($userbank->HasAccess(ADMIN_OWNER)) {
								echo '<a key_phrase="Logs" class="lang" href="./?logs">Logs</a>';
							}
						?>
						<?php
						echo '<a key_phrase="Go Back Link" class="lang" href="'. $GLOBALS['SERVER_FORUM_URL'] .'">Go Back</a>'
						?>
						<?php
							if (!$userbank->HasAccess(ADMIN_OWNER | ADMIN_DELETE_BAN)) {
								echo '<a key_phrase="Login" class="lang" href="./?login">Login</a>';
							}
							else
							{
								echo '<a key_phrase="Logout" class="lang" href="./?logout">Logout</a>';
							}
						?>
					</div>
					<div class="navbar-right">
						<div class="dropdown">
							<button key_phrase="Theme" class="navbar-btn lang">Theme</button>
							<div class="dropdown-content">
								<a class="navbar-lang" onclick="SetTheme('black');" href="#">Black</a>
								<a class="navbar-lang" onclick="SetTheme('white');" href="#">White</a>
							</div>
						</div>
						<div class="dropdown">
							<button key_phrase="Language" class="navbar-btn lang">Language</button>
							<div class="dropdown-content">
								<a class="navbar-lang" onclick="SetLang('en'); localStorage.setItem('lang', 'en');" href="#">ENG</a>
								<a class="navbar-lang" onclick="SetLang('ru'); localStorage.setItem('lang', 'ru');" href="#">RUS</a>
							</div>
						</div>
						<form class="navbar-form" method="get">
							<input type="text" name="search" placeholder="SteamID" class="navbar-input">
							<button type="submit" key_phrase="Find" class="navbar-btn lang">Find</button>
						</form>
					</div>
				</div>
			</nav>
			<div class="data">
				<?php
				if (isset($_GET['login']))
				{
					if (isset($_GET['failed']))
					{
						echo '<center><h1 key_phrase="Authentication failed" class="btn-danger lang">Authentication failed</h1></center>';
					}
					echo '<table style="margin: 30px auto;">
							<tbody>
								<tr>
									<h2 key_phrase="Sourcebans Admin Login" class="lang" style="text-align:center"><b>Sourcebans Admin Login</b></h2>
								</tr>
								<tr>
									<td class="" style="padding: 15px;">
										<div id="login-content">
											<div id="loginUsernameDiv">
												<label key_phrase="Username" for="loginUsername" class="lang">Username</label><br>
												<input id="loginUsername" class="" type="text" name="username" value="">
											</div>
											<div id="loginUsernameMsg" class=""></div>
											<br>
											<div id="loginPasswordDiv">
												<label key_phrase="Password" for="loginPassword" class="lang">Password</label><br>
												<input id="loginPassword" class="" type="password" name="password" value="">
											</div>
											<div id="loginPasswordMsg" class=""></div>
											<br>
											<div id="loginRememberMeDiv">
												<input id="loginRememberMe" type="checkbox" class="checkbox" name="remember" value="checked" vspace="5px">
												<span key_phrase="Remember me" class="checkbox lang" style="cursor:pointer;" onclick="($(\'#loginRememberMe\').is(\':checked\')?$(\'#loginRememberMe\').prop(\'checked\', false):$(\'#loginRememberMe\').prop(\'checked\', true))">
													Remember me
												</span>
											</div>
											<br>
											<div id="loginSubmit">
												<br>
												<button type="button" onclick="DoLogin(\'\');" name="alogin" class="btn btn-info" id="alogin"><span key_phrase="Login" class="lang">Login</span></button>
											</div>														
										</div>
									</td>
								</tr>
							</tbody>
						</table>';
					return;
				}
				if (isset($_GET['logout']))
				{
					Auth::logout();
					header('Location: /');
					exit();
				}
				if (isset($_GET['edit']) && isset($_GET['table_name']) && $userbank->HasAccess(ADMIN_OWNER | ADMIN_EDIT_ALL_BANS))
				{
					$eban_id = $_GET['edit'];
					$table_name = $_GET['table_name'];
					if ($table_name != 'EntWatch_Current_Eban' && $table_name != 'EntWatch_Old_Eban')
						return;

					$GLOBALS['PDO_EBANS']->query("SELECT * FROM ".$table_name." WHERE id = :id");
					$GLOBALS['PDO_EBANS']->bind(':id', $eban_id, PDO::PARAM_INT);
					$row = $GLOBALS['PDO_EBANS']->single();
					?>
					<table class="data-eban-table">
					<thead>
						<tr>
							<td key_phrase="Server" class="lang">Server</td>
							<td key_phrase="Player" class="data-eban-center lang">Player</td>
							<td key_phrase="PlayerSteamID" class="data-eban-center lang">Player SteamID</td>
							<td key_phrase="Reason" class="lang">Reason</td>
							<td key_phrase="Admin" class="data-eban-center lang">Admin</td>
							<td key_phrase="AdminSteamID" class="data-eban-center lang">Admin SteamID</td>
							<td key_phrase="Duration" class="lang">Duration</td>
							<?php
								if ($userbank->HasAccess(ADMIN_OWNER | ADMIN_EDIT_ALL_BANS)) {
									echo '<td key_phrase="Save" class="lang">Save</td>';
								}
							?>
						</tr>
					</thead>
					<tbody>
					<?php 
						$data_id = f_clean_data($row["id"]);
						$data_server = f_clean_data($row["server"]);
						$data_client_name = f_clean_data($row["client_name"]);
						$data_client_steamid = f_clean_data($row["client_steamid"]);
						$data_admin_name = f_clean_data($row["admin_name"]);
						$data_admin_steamid = f_clean_data($row["admin_steamid"]);
						$data_duration = f_clean_data($row["duration"]);
						$data_issued = f_clean_data($row["timestamp_issued"]);
						$data_reason = f_clean_data($row["reason"]);
						$data_unban_admin_name = f_clean_data($row["admin_name_unban"]);
						$data_unban_admin_steamid = f_clean_data($row["admin_steamid_unban"]);
						$data_unban_reason = f_clean_data($row["reason_unban"]);
						$data_unban_time = f_clean_data($row["timestamp_unban"]);
						echo '<td><input id="banservername" value="'.$data_server.'"/></td>
						<td class="data-eban-center"><input id="banplayername" value="'.$data_client_name.'"/></td>
						<td class="data-eban-center data-eban-steamid"><input id="banplayersteamid" value="'.$data_client_steamid.'"/></td>
						<td><input id="banreason" value="'.$data_reason.'"/></td>
						<td class="data-eban-center"><input id="banadminname" value="'.$data_admin_name.'"/></td>
						<td class="data-eban-center data-eban-steamid"><input id="banadminsteamid" value="'.$data_admin_steamid.'"/></td>
						<td class="data-eban-duration">
							<select id="banlength" name="banlength" tabindex="5" class="submit-fields">
								<option value="'.$data_duration.'">Dont change ('.$data_duration.' mins)</option>
								<option value="0">Permanent</option>
								<optgroup label="minutes">
									<option value="1">1 minute</option>
									<option value="5">5 minutes</option>
									<option value="10">10 minutes</option>
									<option value="15">15 minutes</option>
									<option value="30">30 minutes</option>
									<option value="45">45 minutes</option>
								</optgroup><optgroup label="hours">
									<option value="60">1 hour</option>
									<option value="120">2 hours</option>
									<option value="180">3 hours</option>
									<option value="240">4 hours</option>
									<option value="480">8 hours</option>
									<option value="720">12 hours</option>
								</optgroup><optgroup label="days">
									<option value="1440">1 day</option>
									<option value="2880">2 days</option>
									<option value="4320">3 days</option>
									<option value="5760">4 days</option>
									<option value="7200">5 days</option>
									<option value="8640">6 days</option>
								</optgroup><optgroup label="weeks">
									<option value="10080">1 week</option>
									<option value="20160">2 weeks</option>
									<option value="30240">3 weeks</option>
								</optgroup><optgroup label="months">
									<option value="43200">1 month</option>
									<option value="86400">2 months</option>
									<option value="129600">3 months</option>
									<option value="259200">6 months</option>
									<option value="518400">12 months</option>
								</optgroup>
							</select>
						</td>';
						if ($userbank->HasAccess(ADMIN_OWNER | ADMIN_EDIT_ALL_BANS)) {
							echo '<td class="data-eban-center"><button class="save" onclick="SaveBan('.$data_id.', \''.$table_name.'\', \'\')"><span key_phrase="Save" class="lang">Save</span></button></td>';
						}
						echo'</tr>';
					?>
				  </tbody>
				</table>
				<?php
					return;
				}
				if (isset($_GET['logs']) && $userbank->HasAccess(ADMIN_OWNER))
				{
					echo '<table class="data-eban-table">
						<thead>
							<tr>
								<td key_phrase="Title" class="lang">Title</td>
								<td key_phrase="Message" class="data-eban-center lang">Message</td>
								<td key_phrase="Host" class="lang">Host</td>
								<td key_phrase="Admin ID" class="data-eban-center lang">Admin ID</td>
								<td key_phrase="Admin Name" class="data-eban-center lang">Admin Name</td>
								<td key_phrase="Created" class="lang">Created</td>
							</tr>
						</thead>
						<tbody>';
					$GLOBALS['PDO_EBANS']->query("SELECT * FROM `logs` ORDER BY created DESC");
					$rows_logs = $GLOBALS['PDO_EBANS']->resultset();
					foreach ($rows_logs as $row)
					{
						$data_title = f_clean_data($row["title"]);
						$data_message = f_clean_data($row["message"]);
						$data_host = f_clean_data($row["host"]);
						$data_aid = f_clean_data($row["aid"]);
						$data_created = f_clean_data($row["created"]);

						$date = new DateTime();
						$date->setTimestamp($data_created);
						$data_created_formatted = $date->format('Y-m-d H:i:s');

						echo '<td class="data-eban-center">'.$data_title.'</td>';
						echo '<td class="data-eban-center">'.$data_message.'</td>';
						echo '<td class="data-eban-center">'.$data_host.'</td>';
						echo '<td class="data-eban-center">'.$data_aid.'</td>';
						echo '<td class="data-eban-center">'.$userbank->GetUserArray($data_aid)['user'].'</td>';
						echo '<td class="data-eban-center">'.$data_created_formatted.'</td>';
						echo'</tr>';
					}
					echo '</tbody>
						</table>';
					return;
				}
				if($search_state == 1)
				{
					echo '<p key_phrase="Steam Wrong" class="data_search_state_error lang">SteamID is in the wrong format. Supported formats: STEAM_1:0:123456789 and [U:1:123456789]</p>';	
				}elseif($search_state == 2)
				{
					echo '<p class="data_search_state_found"><span key_phrase="Search Results" class="lang">Search Results:</span> '.$buff_steamid.'</p>';
				}
				?>
				<table class="data-eban-table">
					<thead>
						<tr>
							<td key_phrase="Server" class="lang">Server</td>
							<td key_phrase="Player" class="data-eban-center lang">Player</td>
							<td key_phrase="Reason" class="lang">Reason</td>
							<td key_phrase="Admin" class="data-eban-center lang">Admin</td>
							<td key_phrase="Duration" class="lang">Duration</td>
							<?php
								if ($userbank->HasAccess(ADMIN_OWNER | ADMIN_EDIT_ALL_BANS)) {
									echo '<td key_phrase="Edit" class="lang">Edit</td>';
								}
								if ($userbank->HasAccess(ADMIN_OWNER | ADMIN_DELETE_BAN)) {
									echo '<td key_phrase="Delete" class="lang">Delete</td>';
								}
							?>
						</tr>
					</thead>
					<tbody>
					<?php foreach ($rows_alldata as $row)
						{
							$data_id = f_clean_data($row["id"]);
							$data_table_name = f_clean_data($row["table_name"]);
							$data_server = f_clean_data($row["server"]);
							$data_client_name = f_clean_data($row["client_name"]);
							$data_client_steamid = f_clean_data($row["client_steamid"]);
							$data_admin_name = f_clean_data($row["admin_name"]);
							$data_admin_steamid = f_clean_data($row["admin_steamid"]);
							$data_duration = f_clean_data($row["duration"]);
							$data_issued = f_clean_data($row["timestamp_issued"]);
							$data_reason = f_clean_data($row["reason"]);
							$data_unban_admin_name = f_clean_data($row["admin_name_unban"]);
							$data_unban_admin_steamid = f_clean_data($row["admin_steamid_unban"]);
							$data_unban_reason = f_clean_data($row["reason_unban"]);
							$data_unban_time = f_clean_data($row["timestamp_unban"]);
							echo '<tr class="';
							if($data_duration == 0 && $data_unban_admin_steamid == ""){
								echo 'data-eban-permanent" eban-data="Issued: Never">';
							} elseif($data_unban_admin_steamid != ""){
								echo 'data-eban-expired" eban-data="Removed: '.date('m-d-Y H:i:s', $data_unban_time).' Unbanned by: '.$data_unban_admin_name.' ('.$data_unban_admin_steamid.') Reason: '.$data_unban_reason.'">';
							} else {
								echo 'data-eban-active" eban-data="Issued: '.date('m-d-Y H:i:s', $data_issued).'">';
							}
							echo '<td>'.$data_server.'</td>
							<td class="data-eban-center">'.$data_client_name.'<br>(<span class="data-eban-steamid">'.$data_client_steamid.'</span>)</td>
							<td>'.$data_reason.'</td>
							<td class="data-eban-center">'.$data_admin_name.'<br>(<span class="data-eban-steamid">'.$data_admin_steamid.'</span>)</td>
							<td class="';
							if($data_duration == 0 && $data_unban_admin_steamid == ""){
								echo 'data-eban-permanent-duration lang" key_phrase="Permanent">Permanent</td>';
							} elseif($data_unban_admin_steamid != ""){
								if($data_duration == 0){
									echo 'data-eban-expired-duration lang" key_phrase="Permanent Removed">Permanent(Removed)';
								}elseif($data_unban_admin_steamid == "SERVER")
								{
									echo 'data-eban-expired-duration">'.duration_string($data_duration).'(<span key_phrase="Expired" class="lang">Expired</span>)';
								}else
								{
									echo 'data-eban-expired-duration">'.duration_string($data_duration).'(<span key_phrase="Removed" class="lang">Removed</span>)';
								}
								echo '</td>';
							} else {
								echo 'data-eban-active-duration">'.duration_string($data_duration).'</td>';
							}
							if ($userbank->HasAccess(ADMIN_OWNER | ADMIN_EDIT_ALL_BANS)) {
								echo '<td class="data-eban-center"><button class="edit" onclick="location.href=\'./?edit='.$data_id.'&table_name='.$data_table_name.'\'"><span key_phrase="Edit" class="lang">Edit</span></button></td>';
							}
							if ($userbank->HasAccess(ADMIN_OWNER | ADMIN_DELETE_BAN)) {
								echo '<td class="data-eban-center"><button class="delete" onclick="DeleteBan('.$data_id.')"><span key_phrase="Delete" class="lang">Delete</span></button></td>';
							}
						echo'</tr>';
						}?>
				  </tbody>
				</table>
				<div class="data-pages">
					<?php
					if($num_pages<=1) 
					{
						echo '<a class="current-page" href="?page=1">1</a>';
					}else
					{
						for ($i = 1; $i <= $num_pages; $i++)
						{
							if($i==1||($i==$num_pages && $num_pages!=1))
							{
								if($i==$cur_page)
								{
									echo '<a class="current-page" href="?page='.$i.'">'.$i.'</a>';
								}else
								{
									echo '<a href="?page='.$i.'">'.$i.'</a>';
								}
								continue;
							}
							if($i==$cur_page)
							{
								echo '<a class="current-page" href="?page='.$i.'">'.$i.'</a>';
							}
							if($i-1==$cur_page && $i-1>=1)
							{
								echo '<a href="?page='.$i.'">'.$i.'</a>';
							}
							if($i+1==$cur_page && $i+1<=$num_pages)
							{
								echo '<a href="?page='.$i.'">'.$i.'</a>';
							}
							if($i-2==$cur_page && $i-2>=1)
							{
								echo '<a href="#">...</a>';
							}
							if($i+2==$cur_page && $i+2<=$num_pages)
							{
								echo '<a href="#">...</a>';
							}
						}
					}
					?>
				</div>
			</div>
		</div>
		<div class="mobile">
			<nav class="mobile-navbar">
				<p>
					<a key_phrase="EBan List Link" class="lang mobile-navbar-link" href="./?page=1">EBan List</a>
					<a key_phrase="Go Back Link" class="lang mobile-navbar-link" href="https://google.com">Go Back</a>
				</p>
				<p>
					<div class="dropdown">
						<button class="dropbtn">Language</button>
						<div class="dropdown-content">
							<a class="navbar-lang" onclick="SetLang('en'); localStorage.setItem('lang', 'en');" href="#">ENG</a>
							<a class="navbar-lang" onclick="SetLang('ru'); localStorage.setItem('lang', 'ru');" href="#">RUS</a>
						</div>
					</div>
				</p>
				<form method="get">
						<input type="text" name="search" placeholder="SteamID" class="mobile-navbar-input">
						<button type="submit" key_phrase="Find" class="mobile-navbar-btn lang">Find</button>
				</form>
			</nav>
			<div class="mobile-data">
				<?php
				if($search_state == 1)
				{
					echo '<p key_phrase="Steam Wrong" class="mobile-data_search_state_error lang">SteamID is in the wrong format. Supported formats: STEAM_1:0:123456789 and [U:1:123456789]</p>';	
				}elseif($search_state == 2)
				{
					echo '<p class="mobile-data_search_state_found"><span key_phrase="Search Results" class="lang">Search Results:</span> '.$buff_steamid.'</p>';
				}
				?>
				
				<?php foreach ($rows_alldata as $row)
					{
						$data_server = f_clean_data($row["server"]);
						$data_client_name = f_clean_data($row["client_name"]);
						$data_client_steamid = f_clean_data($row["client_steamid"]);
						$data_admin_name = f_clean_data($row["admin_name"]);
						$data_admin_steamid = f_clean_data($row["admin_steamid"]);
						$data_duration = f_clean_data($row["duration"]);
						$data_issued = f_clean_data($row["timestamp_issued"]);
						$data_reason = f_clean_data($row["reason"]);
						$data_unban_admin_name = f_clean_data($row["admin_name_unban"]);
						$data_unban_admin_steamid = f_clean_data($row["admin_steamid_unban"]);
						$data_unban_reason = f_clean_data($row["reason_unban"]);
						$data_unban_time = f_clean_data($row["timestamp_unban"]);
						
						echo '<div class="modile-data-block">';
							echo '<div class="mobile-data-main"><div key_phrase="Server" class="mobile-data-left lang">Server</div><div class="mobile-data-right">'.$data_server.'</div></div>';
							echo '<div class="mobile-data-main"><div key_phrase="Player" class="mobile-data-left lang">Player</div><div class="mobile-data-right">'.$data_client_name.' (<span class="data-eban-steamid">'.$data_client_steamid.'</span>)</div></div>';
							echo '<div class="mobile-data-main"><div key_phrase="Reason" class="mobile-data-left lang">Reason</div><div class="mobile-data-right">'.$data_reason.'</div></div>';
							echo '<div class="mobile-data-main"><div key_phrase="Admin" class="mobile-data-left lang">Admin</div><div class="mobile-data-right">'.$data_admin_name.' (<span class="data-eban-steamid">'.$data_admin_steamid.'</span>)</div></div>';
							echo '<div class="mobile-data-main"><div key_phrase="Duration" class="mobile-data-left lang">Duration</div><div class="mobile-data-right';
							if($data_duration == 0 && $data_unban_admin_steamid == ""){
								echo ' lang" key_phrase="Permanent">Permanent';
							} elseif($data_unban_admin_steamid != ""){
								if($data_duration == 0){
									echo ' lang" key_phrase="Permanent Removed">Permanent(Removed)';
								}elseif($data_unban_admin_steamid == "SERVER")
								{
									echo '">'.duration_string($data_duration).'(<span key_phrase="Expired" class="lang">Expired</span>)';
								}else
								{
									echo '">'.duration_string($data_duration).'(<span key_phrase="Removed" class="lang">Removed</span>)';
								}
							} else {
								echo '">'.duration_string($data_duration);
							}
							echo '</div></div>';
							if($data_duration == 0 && $data_unban_admin_steamid == ""){
								echo '<div class="mobile-data-secondary-permanent"><div key_phrase="Issued" class="mobile-data-left lang">Issued:</div><div key_phrase="Never" class="mobile-data-right lang">Never</div></div>';
							} elseif($data_unban_admin_steamid != ""){
								echo '<div class="mobile-data-secondary-expired"><div key_phrase="Removed_2" class="mobile-data-left lang">Removed:</div><div class="mobile-data-right">'.date('m-d-Y H:i:s', $data_unban_time).'</div></div>';
								echo '<div class="mobile-data-secondary-expired"><div key_phrase="Unbanned by" class="mobile-data-left lang">Unbanned by:</div><div class="mobile-data-right">'.$data_unban_admin_name.' (<span class="data-eban-steamid">'.$data_unban_admin_steamid.'</span>)</div></div>';
								echo '<div class="mobile-data-secondary-expired"><div key_phrase="Reason_2" class="mobile-data-left lang">Reason:</div><div class="mobile-data-right">'.$data_unban_reason.'</div></div>';
							} else {
								echo '<div class="mobile-data-secondary-active"><div key_phrase="Issued" class="mobile-data-left lang">Issued:</div><div class="mobile-data-right">'.date('m-d-Y H:i:s', $data_issued).'</div></div>';
							}
						echo '</div>';
						
					}?>

				<div class="mobile-data-pages">
					<?php
					if($num_pages<=1) 
					{
						echo '<a class="current-page" href="?page=1">1</a>';
					}else
					{
						for ($i = 1; $i <= $num_pages; $i++)
						{
							if($i==1||($i==$num_pages && $num_pages!=1))
							{
								if($i==$cur_page)
								{
									echo '<a class="current-page" href="?page='.$i.'">'.$i.'</a>';
								}else
								{
									echo '<a href="?page='.$i.'">'.$i.'</a>';
								}
								continue;
							}
							if($i==$cur_page)
							{
								echo '<a class="current-page" href="?page='.$i.'">'.$i.'</a>';
							}
							if($i-1==$cur_page && $i-1>=1)
							{
								echo '<a href="?page='.$i.'">'.$i.'</a>';
							}
							if($i+1==$cur_page && $i+1<=$num_pages)
							{
								echo '<a href="?page='.$i.'">'.$i.'</a>';
							}
							if($i-2==$cur_page && $i-2>=1)
							{
								echo '<a href="#">...</a>';
							}
							if($i+2==$cur_page && $i+2<=$num_pages)
							{
								echo '<a href="#">...</a>';
							}
						}
					}
					?>
				</div>
			</div>
		</div>
	</body>
</html>