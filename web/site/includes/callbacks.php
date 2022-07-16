<?php

require_once 'xajax.inc.php';
$xajax = new xajax();
//$xajax->debugOn();
// $xajax->setRequestURI('./index.php');
global $userbank;

if ($userbank->is_admin()) {
    $xajax->registerFunction("SaveBan");
    $xajax->registerFunction("DeleteBan");
}

$xajax->registerFunction("Plogin");

/**
 * @param  string $username
 * @param  string $password
 * @param  string $remember
 * @param  string $redirect
 * @return xajaxResponse
 */
function Plogin(string $username, string $password, string $remember = '', string $redirect = '')
{
    global $userbank;
    $objResponse = new xajaxResponse();

    if (empty($password)) {
        $objResponse->addRedirect('?login&failed', 0);
        return $objResponse;
    }

    $remember = ($remember === 'true') ? true : false;

    $auth = new NormalAuthHandler($GLOBALS['PDO'], $username, $password, $remember);

    if (!$auth->getResult()) {
        $objResponse->addRedirect("?login&failed",  0);
        return $objResponse;
    }

    $objResponse->addRedirect("?".$redirect,  0);
    return $objResponse;
}

/**
 * @param  int $ban_id
 * @return xajaxResponse
 */
function DeleteBan($ban_id)
{
    $objResponse = new xajaxResponse();

    if (empty($ban_id)) {
        $objResponse->addRedirect('?delete&failed', 0);
        return $objResponse;
    }

    $GLOBALS['PDO_EBANS']->query("(SELECT * FROM EntWatch_Current_Eban WHERE id = :id1) UNION ALL (SELECT * FROM EntWatch_Old_Eban WHERE id = :id2)");
    $GLOBALS['PDO_EBANS']->bind(':id1', $ban_id, PDO::PARAM_INT);
    $GLOBALS['PDO_EBANS']->bind(':id2', $ban_id, PDO::PARAM_INT);
    $row = $GLOBALS['PDO_EBANS']->single();

    $data_server = f_clean_data($row["server"]);
    $data_client_name = f_clean_data($row["client_name"]);
    $data_client_steamid = f_clean_data($row["client_steamid"]);
    $data_admin_name = f_clean_data($row["admin_name"]);
    $data_admin_steamid = f_clean_data($row["admin_steamid"]);
    $data_duration = f_clean_data($row["duration"]);
    $data_issued = f_clean_data($row["timestamp_issued"]);
    $data_reason = f_clean_data($row["reason"]);

    $title = "Delete Ban";
    $message = "Server: ".$data_server.", Player: ".$data_client_name.", Player Steam Id: ".$data_client_steamid.", Admin: ".$data_admin_name.", Admin Steam Id: ".$data_admin_steamid.", Duration: ".$data_duration.", Issued: ".$data_issued.", Reason: ".$data_reason;

    AddLog($title, $message);

    $GLOBALS['PDO_EBANS']->query("DELETE FROM `EntWatch_Current_Eban` WHERE id = :id1; DELETE FROM `EntWatch_Old_Eban` WHERE id = :id2;");
    $GLOBALS['PDO_EBANS']->bind(':id1', $ban_id, PDO::PARAM_INT);
    $GLOBALS['PDO_EBANS']->bind(':id2', $ban_id, PDO::PARAM_INT);
    $GLOBALS['PDO_EBANS']->execute();

    $objResponse->addRedirect("?deleted",  0);
    return $objResponse;
}

/**
 * @param  int $ban_id
 * @param  string $table_name
 * @param  string $input_server
 * @param  string $input_client_name
 * @param  string $input_client_steamid
 * @param  string $input_reason
 * @param  string $input_admin_name
 * @param  string $input_admin_steamid
 * @param  int $input_duration
 * @return xajaxResponse
 */
function SaveBan($ban_id, $table_name, $input_server, $input_client_name, $input_client_steamid, $input_reason, $input_admin_name, $input_admin_steamid, $input_duration)
{
    $objResponse = new xajaxResponse();

    if (empty($ban_id) || empty($table_name) || ($table_name != 'EntWatch_Current_Eban' && $table_name != 'EntWatch_Old_Eban')
    || empty($input_server) || empty($input_client_name) || empty($input_client_steamid)
    || empty($input_reason) || empty($input_admin_name) || empty($input_admin_steamid)) {
        $objResponse->addRedirect('?edit='.$ban_id.'&failed', 0);
        return $objResponse;
    }

    $GLOBALS['PDO_EBANS']->query("UPDATE ".$table_name." SET server = :input_server, client_name = :input_client_name, client_steamid = :input_client_steamid, reason = :input_reason, admin_name = :input_admin_name, admin_steamid = :input_admin_steamid, duration = :input_duration WHERE id = :eban_id");
    $GLOBALS['PDO_EBANS']->bind(':eban_id', $ban_id, PDO::PARAM_INT);
    $GLOBALS['PDO_EBANS']->bind(':input_server', filter_var($input_server, FILTER_SANITIZE_STRING), PDO::PARAM_STR);
    $GLOBALS['PDO_EBANS']->bind(':input_client_name', filter_var($input_client_name, FILTER_SANITIZE_STRING), PDO::PARAM_STR);
    $GLOBALS['PDO_EBANS']->bind(':input_client_steamid', filter_var($input_client_steamid, FILTER_SANITIZE_STRING), PDO::PARAM_STR);
    $GLOBALS['PDO_EBANS']->bind(':input_reason', filter_var($input_reason, FILTER_SANITIZE_STRING), PDO::PARAM_STR);
    $GLOBALS['PDO_EBANS']->bind(':input_admin_name', filter_var($input_admin_name, FILTER_SANITIZE_STRING), PDO::PARAM_STR);
    $GLOBALS['PDO_EBANS']->bind(':input_admin_steamid', filter_var($input_admin_steamid, FILTER_SANITIZE_STRING), PDO::PARAM_STR);
    $GLOBALS['PDO_EBANS']->bind(':input_duration', $input_duration, PDO::PARAM_INT);
    $GLOBALS['PDO_EBANS']->execute();

    $title = "Edit Ban";
    $message = "Server: ".$input_server.", Player: ".$input_client_name.", Player Steam Id: ".$input_client_steamid.", Admin: ".$input_admin_name.", Admin Steam Id: ".$input_admin_steamid.", Duration: ".$input_duration.", Issued: ".$input_issued.", Reason: ".$input_reason;

    AddLog($title, $message);

    $objResponse->addRedirect('?edited='.$ban_id.'',  0);
    return $objResponse;
}

function AddLog($title, $message)
{
    global $userbank;

    $GLOBALS['PDO_EBANS']->query("INSERT INTO `logs` (title, message, host, aid, created) VALUES (:title, :message, :host, :aid, :created)");
    $GLOBALS['PDO_EBANS']->bind(':title', filter_var($title, FILTER_SANITIZE_STRING), PDO::PARAM_STR);
    $GLOBALS['PDO_EBANS']->bind(':message', filter_var($message, FILTER_SANITIZE_STRING), PDO::PARAM_STR);
    $GLOBALS['PDO_EBANS']->bind(':host', filter_var($_SERVER['REMOTE_ADDR'], FILTER_VALIDATE_IP) ? $_SERVER['REMOTE_ADDR'] : '', PDO::PARAM_STR);
    $GLOBALS['PDO_EBANS']->bind(':aid', $userbank->GetAid(), PDO::PARAM_INT);
    $GLOBALS['PDO_EBANS']->bind(':created', time(), PDO::PARAM_INT);
    $GLOBALS['PDO_EBANS']->execute();
}
