<?php
function errCallBack($err)
{
    echo "<pre>gm:\n".$err->getMessage()."\ndi:\n".$err->getDebugInfo()."\nui:\n".$err->getUserInfo()."\n";
    echo "<hr>BackTrace:\n";
    print_r($err->backtrace);
    echo "</pre>\n";
    exit;
}

// --- basic funtionality ---
/**
 *  tra
 *
 *  Translate the given string using localisation files.
 *
 *  @param input string, string to translate
 *  @return string, translated string
 */
function tra($input)
{
    // just a dummy function yet
    $nr=func_num_args();
    if ($nr>1)
    for ($i=1; $i<$nr; $i++){
        $name  = '$'.$i;
        $val   = func_get_arg($i);
        $input = str_replace($name, $val, $input);
    }
    return $input;
}


function _getDArr($format)
{
    #$arr['']  = '00';
    switch($format) {
    case 'h':
        for($n=0; $n<=23; $n++) {
            $arr[$this->_twoDigits($n)] = $this->_twoDigits($n);
        }
        break;

    case 'm':
    case 's':
        for($n=0; $n<=59; $n++) {
            $arr[$this->_twoDigits($n)] = $this->_twoDigits($n);
        }
        break;
    }

    return $arr;
}


/**
 *  uiBase class
 *
 *  LiveSupport HTML User Interface module
 *
 */
class uiBase
{
    // --- class constructor ---
    /**
     *  uiBase
     *
     *  Initialize a new Basis Class including:
     *  - database  initialation
     *  - GreenBox initialation
     *
     *  @param $config array, configurartion data
     */
    var $redirUrl;
    var $alertMsg;

    function uiBase(&$config)
    {
        $this->dbc = DB::connect($config['dsn'], TRUE);
        if (DB::isError($this->dbc)) {
            die($this->dbc->getMessage());
        }
        $this->dbc->setFetchMode(DB_FETCHMODE_ASSOC);
        $this->gb       =& new GreenBox($this->dbc, $config);
        $this->config   =& $config;
        $this->config['accessRawAudioUrl'] = $config['storageUrlPath'].'/xmlrpc/simpleGet.php';
        $this->sessid   = $_REQUEST[$config['authCookieName']];
        $this->userid   = $this->gb->getSessUserId($this->sessid);
        $this->login    = $this->gb->getSessLogin($this->sessid);
        $this->id       = $_REQUEST['id'] ? $_REQUEST['id'] : $this->gb->getObjId($this->login, $this->gb->storId);
        $this->pid      = $this->gb->getparent($this->id) != 1 ? $this->gb->getparent($this->id) : FALSE;
        $this->type     = $this->gb->getFileType($this->id);
        $this->fid      = $this->type=='Folder' ? $this->id : $this->pid;
        $this->homeid   = $this->gb->getObjId($this->login, $this->gb->storId);
        $this->InputTextStandardAttrib = array('size'     =>UI_INPUT_STANDARD_SIZE,
                                               'maxlength'=>UI_INPUT_STANDARD_MAXLENGTH);
        $this->STATIONPREFS =& $_SESSION[UI_STATIONINFO_SESSNAME];
        $this->SCRATCHPAD   =& new uiScratchPad($this);
        $this->SEARCH       =& new uiSearch($this);
        $this->PLAYLIST     =& new uiPlaylist($this);
    }



    function loadStationPrefs(&$mask)
    {
        if (!is_array($this->STATIONPREFS)) {
            foreach ($mask as $key=>$val) {
                if ($val['isPref']) {
                    if (is_string($setting = $this->gb->loadGroupPref(NULL, 'StationPrefs', $val['element']))) {
                        $this->STATIONPREFS[$val['element']] = $setting;
                    } else {
                        $miss = TRUE;
                    }
                }
            }
            if (!$this->STATIONPREFS['stationMaxfilesize'])
                $this->STATIONPREFS['stationMaxfilesize'] = strtr(ini_get('upload_max_filesize'), array('M'=>'000000', 'k'=>'000'));
            /*
            if ($miss && $this->login) {
                $this->_retMsg('Note: Station Preferences not setup.');
            }
            */
        }
    }

    /**
     *  _parseArr2Form
     *
     *  Add elements/rules/groups to an given HTML_QuickForm object
     *
     *  @param form object, reference to HTML_QuickForm object
     *  @param mask array, reference to array defining to form elements
     *  @param side string, side where the validation should beeing
     */
    function _parseArr2Form(&$form, &$mask, $side='client')
    {
        foreach($mask as $k=>$v) {
            ## add elements ########################
            if ($v['type']=='radio') {
                foreach($v['options'] as $rk=>$rv) {
                    $radio[] =& $form->createElement($v['type'], NULL, NULL, $rv, $rk, $v['attributes']);
                }
                $form->addGroup($radio, $v['element'], tra($v['label']));
                unset($radio);

            } elseif ($v['type']=='select') {
                $elem[$v['element']] =& $form->createElement($v['type'], $v['element'], tra($v['label']), $v['options'], $v['attributes']);
                $elem[$v['element']]->setMultiple($v['multiple']);
                if (isset($v['selected'])) $elem[$v['element']]->setSelected($v['selected']);
                if (!$v['groupit'])        $form->addElement($elem[$v['element']]);

            } elseif ($v['type']=='date') {
                $elem[$v['element']] =& $form->createElement($v['type'], $v['element'], tra($v['label']), $v['options'], $v['attributes']);
                if (!$v['groupit'])     $form->addElement($elem[$v['element']]);

            } elseif ($v['type']=='checkbox' || $v['type']=='static') {
                $elem[$v['element']] =& $form->createElement($v['type'], $v['element'], tra($v['label']), $v['text'], $v['attributes']);
                if (!$v['groupit'])     $form->addElement($elem[$v['element']]);

            } elseif (isset($v['type'])) {
                $elem[$v['element']] =& $form->createElement($v['type'], $v['element'], tra($v['label']),
                                            ($v[type]=='text' || $v['type']=='file' || $v['type']=='password') ? array_merge($v['attributes'], array('size'=>UI_INPUT_STANDARD_SIZE, 'maxlength'=>UI_INPUT_STANDARD_MAXLENGTH)) :
                                            ($v['type']=='textarea' ? array_merge($v['attributes'], array('rows'=>UI_TEXTAREA_STANDART_ROWS, 'cols'=>UI_TEXTAREA_STANDART_COLS)) : $v['attributes'])
                                        );
                if (!$v['groupit'])     $form->addElement($elem[$v['element']]);
            }
            ## add required rule ###################
            if ($v['required']) {
                $form->addRule($v['element'], isset($v['requiredmsg'])?tra($v['requiredmsg']):tra('Missing value for $1', tra($v['label'])), 'required', NULL, $side);
            }
            ## add constant value ##################
            if (isset($v['constant'])) {
                $form->setConstants(array($v['element']=>$v['constant']));
            }
            ## add default value ###################
            if (isset($v['default'])) {
                $form->setDefaults(array($v['element']=>$v['default']));
            }
            ## add other rules #####################
            if ($v['rule']) {
                $form->addRule($v['element'], isset($v['rulemsg']) ? tra($v['rulemsg']) : tra('$1 must be $2', tra($v['element']), tra($v['rule'])), $v['rule'] ,$v['format'], $side);
            }
            ## add group ###########################
            if (is_array($v['group'])) {
                foreach($v['group'] as $val) {
                    $groupthose[] =& $elem[$val];
                }
                $form->addGroup($groupthose, $v['name'], tra($v['label']), $v['seperator'], $v['appendName']);
                if ($v['rule']) {
                    $form->addRule($v['name'], isset($v['rulemsg']) ? tra($v['rulemsg']) : tra('$1 must be $2', tra($v['name'])), $v['rule'], $v['format'], $side);
                }
                if ($v['grouprule']) {
                    $form->addGroupRule($v['name'], $v['arg1'], $v['grouprule'], $v['format'], $v['howmany'], $side, $v['reset']);
                }
                unset($groupthose);
            }
            ## check error on type file ##########
            if ($v['type']=='file') {
                if ($_POST[$v['element']]['error']) {
                    $form->setElementError($v['element'], isset($v['requiredmsg']) ? tra($v['requiredmsg']) : tra('Missing value for $1', tra($v['label'])));
                }
            }
        }

        reset($mask);
        $form->validate();
    }


    /**
     *  _dateArr2Str
     *
     *  Converts date-array from form into string
     *
     *  @param input array, array of form-elements
     */
    function _dateArr2Str(&$input)
    {
        foreach ($input as $k=>$v){
            if (is_array($v)) {
                if ( ( isset($v['d']) ) && ( isset($v['M']) || isset($v['m']) ) && ( isset($v['Y']) || isset($v['y']) ) ) {
                    $input[$k] = $v['Y'].$v['y'].'-'.$this->_twoDigits($v['M'].$v['m']).'-'.$this->_twoDigits($v['d']);
                }
                if ( ( isset($v['H']) ) || isset($v['h'] ) && ( isset($v['i']) ) && ( isset($v['s']) ) ) {
                    $input[$k] = $this->_twoDigits($v['H'].$v['h']).':'.$this->_twoDigits($v['i']).':'.$this->_twoDigits($v['s']);
                }
            }
        }

        return $input;
    }


    /**
     *  _analyzeFile
     *
     *  Call getid3 library to analyze media file and show some results
     *
     *  @param $id int local ID of file
     *  @param $format string
     */
    function _analyzeFile($id, $format)
    {
        $ia = $this->gb->analyzeFile($id, $this->sessid);
        $s  = $ia['playtime_seconds'];
        $extent = date('H:i:s', floor($s)-date('Z')).substr(number_format($s, 6), strpos(number_format($s, 6), '.'));

        if ($format=='array') {
            return array(
                    'Format.Extent'             => $extent,
                    'Format.Medium.Bitrate'     => $ia['audio']['bitrate'],
                    'Format.Medium.Channels'    => $ia['audio']['channelmode'],
                    'Format.Medium.Samplerate'  => $ia['audio']['sample_rate'],
                    'Format.Medium.Encoder'     => $ia['audio']['codec'] ? $ia['audio']['codec'] : $ia['audio']['encoder'],
                   );
        } elseif ($format=='text') {
            #print_r($ia);
            return "fileformat: {$ia['fileformat']}<br>
                    seconds: {$ia['playtime_seconds']}<br>
                    length: $extent<br>
                    channels: {$ia['audio']['channels']}<br>
                    sample_rate: {$ia['audio']['sample_rate']}<br>
                    bits_per_sample: {$ia['audio']['bits_per_sample']}<br>
                    channelmode: {$ia['audio']['channelmode']}<br>
                    title: {$ia['id3v1']['title']}<br>
                    artist: {$ia['id3v1']['artist']}<br>
                    comment: {$ia['id3v1']['comment']}";
        } elseif ($format=='xml') {
            return
                  '<?xml version="1.0" encoding="utf-8"?>
                  <audioClip>
                  <metadata
                    xmlns="http://www.streamonthefly.org/"
                    xmlns:dc="http://purl.org/dc/elements/1.1/"
                    xmlns:dcterms="http://purl.org/dc/terms/"
                    xmlns:xbmf="http://www.streamonthefly.org/xbmf"
                    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                   >
                  <dc:title>'.$this->_getFileTitle($id).'</dc:title>
                  <dcterms:extent>'.date('H:i:s', round($s)-date('Z')).substr(number_format($s, 6), strpos(number_format($s, 6), '.')).'</dcterms:extent>
                  </metadata>
                  </audioClip>';

        }
    }


    function _twoDigits($num)
    {
        if (strlen($num)<2)
            return ("0$num");
        else
            return $num;
    }


    function _toHex($gunid)
    {
        $res = $this->dbc->query("SELECT to_hex($gunid)");
        $row = $res->fetchRow();

        return $row['to_hex'];
    }


    function _toInt8($gunid)
    {
        $res = $this->dbc->query("SELECT x'$gunid'::bigint");
        $row = $res->fetchRow();

        return $row['int8'];
    }


    function _retMsg($msg, $p1=NULL, $p2=NULL, $p3=NULL, $p4=NULL, $p5=NULL, $p6=NULL, $p7=NULL, $p8=NULL, $p9=NULL)
    {
        $_SESSION['alertMsg'] .= tra($msg, $p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9).'\n';
    }


    function _getMetaInfo($id)
    {
        $data = array('id'          => $id,
                      'gunid'       => $this->gb->_gunidFromId($id),
                      'title'       => $this->_getMDataValue($id, 'title'),
                      'creator'     => $this->_getMDataValue($id, 'creator'),
                      'duration'    => $this->_niceTime($this->_getMDataValue($id, 'dcterms:extent')),
                      'type'        => $this->gb->getFileType($id),
                );
         return ($data);
    }


    function _niceTime($in)
    {
        list ($in, $lost) = explode('.', $in);
        if (preg_match('/^[0-9]{1,2}:[0-9]{1,2}:[0-9]{1,2}$/', $in))
            list($h, $i, $s) = explode(':', $in);
        elseif (preg_match('/^[0-9]{1,2}:[0-9]{1,2}$/', $in))
            list($i, $s) = explode(':', $in);
        else
            $s = $in;

        if ($h > 0) {
            $H = $this->_twoDigits($h).':';
        } else {
            $H = '&nbsp;&nbsp;&nbsp;';
        }

        $I = $this->_twoDigits($i).':';
        $S = $this->_twoDigits($s);

        return $H.$I.$S;
    }


    function _getMDataValue($id, $key)
    {
        if (is_array($arr = $this->gb->getMDataValue($id, $key, $this->sessid))) {
            $value = array_pop($arr);
            return $value['value'];
        }
        return FALSE;
    }


    function _getFileTitle($id)
    {
        if (is_array($arr = $this->gb->getPath($id))) {
            $file = array_pop($arr);
            return $file['name'];
        }
        return FALSE;
    }


    function _isFolder($id)
    {
        if (strtolower($this->gb->getFileType($id)) != 'folder') {
            return FALSE;
        }
        return TRUE;
    }


    function _formElementEncode($str)
    {
        $str = str_replace(':', '__', $str);
        $str = str_replace('.', '_', $str);
        return $str;
    }


    function _formElementDecode($str)
    {
        $str = str_replace('__', ':', $str);
        $str = str_replace('_', '.', $str);
        return $str;
    }
}
?>