<?php
function errCallBack($err)
{
    echo "<pre>gm:\n".$err->getMessage()."\ndi:\n".$err->getDebugInfo()."\nui:\n".$err->getUserInfo()."\n";
    echo "<hr>BackTrace:\n";
    print_r($err->backtrace);
    echo "</pre>\n";
    exit;
}

/**
 *  uiBase class
 *
 *  LiveSupport HTML User Interface module
 *
 */
class uiBase
{
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
                $form->addGroup($radio, $v['element'], $this->tra($v['label']));
                unset($radio);

            } elseif ($v['type']=='select') {
                $elem[$v['element']] =& $form->createElement($v['type'], $v['element'], $this->tra($v['label']), $v['options'], $v['attributes']);
                $elem[$v['element']]->setMultiple($v['multiple']);
                if (isset($v['selected'])) $elem[$v['element']]->setSelected($v['selected']);
                if (!$v['groupit'])        $form->addElement($elem[$v['element']]);

            } elseif ($v['type']=='date') {
                $elem[$v['element']] =& $form->createElement($v['type'], $v['element'], $this->tra($v['label']), $v['options'], $v['attributes']);
                if (!$v['groupit'])     $form->addElement($elem[$v['element']]);

            } elseif ($v['type']=='checkbox' || $v['type']=='static') {
                $elem[$v['element']] =& $form->createElement($v['type'], $v['element'], $this->tra($v['label']), $v['text'], $v['attributes']);
                if (!$v['groupit'])     $form->addElement($elem[$v['element']]);

            } elseif (isset($v['type'])) {
                $elem[$v['element']] =& $form->createElement($v['type'], $v['element'], $this->tra($v['label']),
                                            ($v[type]=='text' || $v['type']=='file' || $v['type']=='password') ? array_merge($v['attributes'], array('size'=>UI_INPUT_STANDARD_SIZE, 'maxlength'=>UI_INPUT_STANDARD_MAXLENGTH)) :
                                            ($v['type']=='textarea'                  ? array_merge($v['attributes'], array('rows'=>UI_TEXTAREA_STANDART_ROWS, 'cols'=>UI_TEXTAREA_STANDART_COLS)) : $v['attributes'])
                                        );
                if (!$v['groupit'])     $form->addElement($elem[$v['element']]);
            }
            ## add required rule ###################
            if ($v['required']) {
                $form->addRule($v['element'], isset($v['requiredmsg'])?$this->tra($v['requiredmsg']):$this->tra('Missing value for $1', $this->tra($v['label'])), 'required', NULL, $side);
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
                $form->addRule($v['element'], isset($v['rulemsg']) ? $this->tra($v['rulemsg']) : $this->tra('$1 must be $2', $this->tra($v['element']), $this->tra($v['rule'])), $v['rule'] ,$v['format'], $side);
            }
            ## add group ###########################
            if (is_array($v['group'])) {
                foreach($v['group'] as $val) {
                    $groupthose[] =& $elem[$val];
                }
                $form->addGroup($groupthose, $v['name'], $this->tra($v['label']), $v['seperator'], $v['appendName']);
                unset($groupthose);
            }
            ## check error on type file ##########
            if ($v['type']=='file') {
                if ($_POST[$v['element']]['error']) {
                    $form->setElementError($v['element'], isset($v['requiredmsg']) ? $this->tra($v['requiredmsg']) : $this->tra('Missing value for $1', $this->tra($v['label'])));
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
     *  @param input array, reference to array of form-elements
     */
    function _dateArr2Str(&$input)
    {
        foreach ($input as $k=>$v){
            if (is_array($v) && isset($v['d']) && (isset($v['M']) || isset($v['m'])) && (isset($v['Y']) || isset($v['y']))){
                $input[$k] = $v['Y'].$v['y'].'-'.(strlen($v['M'].$v['m'])==2 ? $v['M'].$v['m'] : '0'.$v['M'].$v['m']).'-'.(strlen($v['d'])==2 ? $v['d'] : '0'.$v['d']);
            }
        }
    }


    /**
     *  getInfo
     *
     *  Call getid3 library to analyze media file and show some results
     *
     *  @param $id int local ID of file
     *  @param $format string
     */
    function getInfo($id, $format)
    {
        $ia = $this->gb->analyzeFile($id, $this->sessid);

        if ($format=='array') {
            return array(
                    'Format.Extent'             => $ia['playtime_seconds'],
                    'Format.Medium.Bitrate'     => $ia['audio']['bitrate'],
                    'Format.Medium.Channels'    => $ia['audio']['channelmode'],
                    'Format.Medium.Samplerate'  => $ia['audio']['sample_rate'],
                    'Format.Medium.Encoder'     => $ia['audio']['codec'] ? $ia['audio']['codec'] : $ia['audio']['encoder'],
                   );
        } elseif ($format=='text') {
            return "fileformat: {$ia['fileformat']}<br>
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
                  <dc:title  >taken from test xml</dc:title>
                  <dcterms:extent  >00:30:00.000000</dcterms:extent>
                  </metadata>
                  </audioClip>';

        }
    }
}
?>
