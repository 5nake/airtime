<?php

// Define path to application directory
defined('APPLICATION_PATH')
    || define('APPLICATION_PATH', realpath(dirname(__FILE__) . '/../../../install_minimal/../airtime_mvc/application'));

// Ensure library/ is on include_path
set_include_path(implode(PATH_SEPARATOR, array(
    get_include_path(),
    realpath(APPLICATION_PATH . '/../library')
)));

//Propel classes.
set_include_path(APPLICATION_PATH . '/models' . PATH_SEPARATOR . get_include_path());
set_include_path(APPLICATION_PATH . '/models/airtime' . PATH_SEPARATOR . get_include_path());
set_include_path(APPLICATION_PATH . '/models/om' . PATH_SEPARATOR . get_include_path());

require_once 'CcMusicDirsQuery.php';
require_once 'BaseCcMusicDirsQuery.php';

class StorageQuotaUpgrade
{
    public static function startUpgrade()
    {
        echo "* Updating storage usage for new quota tracking".PHP_EOL;
        self::setStorageUsage();
    }

    private static function setStorageUsage()
    {
        $musicDir = CcMusicDirsQuery::create()
            ->filterByDbType('stor')
            ->filterByDbExists(true)
            ->findOne();
        $storPath = $musicDir->getDbDirectory();

        $freeSpace = disk_free_space($storPath);
        $totalSpace = disk_total_space($storPath);

        Application_Model_Preference::setDiskUsage($totalSpace - $freeSpace);
    }
}
