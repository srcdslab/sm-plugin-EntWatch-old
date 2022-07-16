<?php

// ---------------------------------------------------
//  Directories
// ---------------------------------------------------
define('ROOT', dirname(__FILE__) . "/");
define('INCLUDES_PATH', ROOT . 'includes');

if (!file_exists(ROOT.'/config.php')) {
    die('Missing config.php.');
}
require_once(ROOT.'/config.php');
require_once(INCLUDES_PATH.'/Database.php');

$GLOBALS['PDO'] = new Database(SBPP_DB_HOST, SBPP_DB_PORT, SBPP_DB_NAME, SBPP_DB_USER, SBPP_DB_PASS, SBPP_DB_PREFIX, SBPP_DB_CHARSET);
$GLOBALS['PDO_EBANS'] = new Database(EBAN_DB_HOST, EBAN_DB_PORT, EBAN_DB_NAME, EBAN_DB_USER, EBAN_DB_PASS, EBAN_DB_PREFIX, EBAN_DB_CHARSET);
$GLOBALS['SERVER_FORUM_URL'] = SERVER_FORUM_URL;

#Composer autoload
if (!file_exists(INCLUDES_PATH.'/vendor/autoload.php')) {
    die('Compose autoload not found!');
}
require_once(INCLUDES_PATH.'/vendor/autoload.php');

require_once(INCLUDES_PATH.'/security/Crypto.php');
require_once(INCLUDES_PATH.'/auth/JWT.php');

require_once(INCLUDES_PATH.'/auth/Auth.php');
require_once(INCLUDES_PATH.'/auth/Host.php');
require_once(INCLUDES_PATH.'/auth/handler/NormalAuthHandler.php');
require_once(INCLUDES_PATH.'/CUserManager.php');

Auth::init($GLOBALS['PDO']);

$userbank = new CUserManager(Auth::verify());

global $userbank;

$webflags = json_decode(file_get_contents(ROOT.'/configs/permissions/web.json'), true);
foreach ($webflags as $flag => $perm) {
    define($flag, $perm['value']);
}

require_once(INCLUDES_PATH.'/Config.php');
Config::init($GLOBALS['PDO']);
