<?php
class uiPlaylist
{
    function uiPlaylist(&$uiBase)
    {
        $this->Base      =& $uiBase;
        $this->activeId  =& $_SESSION[UI_PLAYLIST_SESSNAME]['activeId'];
        $this->token     =& $_SESSION[UI_PLAYLIST_SESSNAME]['token'];
        $this->reloadUrl = UI_BROWSER.'?popup[]=_reload_parent&popup[]=_close';
    }

    function setReload()
    {
        $this->Base->redirUrl = $this->reloadUrl;
    }

    function get()
    {
        if (!$this->activeId) {
            return FALSE;
        }
        #echo '<pre><div align="left">'; print_r( $this->Base->gb->getPlaylistArray($this->activeId, $this->Base->sessid)); echo '</div></pre>';
        return $this->Base->gb->getPlaylistArray($this->activeId, $this->Base->sessid);
    }

    function getActiveId()
    {
        if (!$this->activeId) {
            return FALSE;
        }
        return $this->activeId;
    }

    function activate($plid, $msg=TRUE)
    {
        # test if PL available
        # look PL
        # store access token to ls_pref abd session
        # load PL into session
        if($this->token) {
             if (UI_WARNING) $this->Base->_retMsg('You have an Playlist already activated,\n first close it');
            return FALSE;
        }
        if(($userid = $this->Base->gb->playlistIsAvailable($plid, $this->Base->sessid)) !== TRUE) {
             if (UI_WARNING) $this->Base->_retMsg('Playlist is looked by $1', $this->Base->gb->getSubjName($userid));
            return FALSE;
        }
        $this->token = $this->Base->gb->lockPlaylistForEdit($plid, $this->Base->sessid);
        $this->Base->gb->savePref($this->Base->sessid, UI_PL_ACCESSTOKEN_KEY, $plid.':'.$this->token);
        $this->activeId = $plid;
        if ($msg) $this->Base->_retMsg('Playlist "$1" activated', $this->Base->_getMDataValue($plid, UI_MDATA_KEY_TITLE));
        return TRUE;
    }

    function release($msg=TRUE)
    {
        # get token from ls_pref
        # release PL
        # delete PL from session
        # remove token from ls_pref
        if(!$this->token) {
             if (UI_WARNING) $this->Base->_retMsg('No Playlist is looked by You');
            return FALSE;
        }
        $plgunid = $this->Base->gb->releaseLockedPlaylist($this->token, $this->Base->sessid);
        if (PEAR::isError($plgunid)) {
            if (UI_VERBOSE) print_r($plgunid);
             if (UI_WARNING) $this->Base->_retMsg('Unable to release Playlist');
            return FALSE;
        }
        if($msg) $this->Base->_retMsg('Playlist "$1" released', $this->Base->_getMDataValue($this->Base->gb->_idFromGunid($plgunid), UI_MDATA_KEY_TITLE));
        $this->activeId = NULL;
        $this->token    = NULL;
        $this->Base->gb->delPref($this->Base->sessid, UI_PL_ACCESSTOKEN_KEY);
        return TRUE;
    }


    function save()
    {
        $tmpid = $this->activeId;
        $this->release(FALSE);
        $this->activate($tmpid, FALSE);
        $this->Base->_retMsg('Playlist "$1" saved', $this->Base->_getMDataValue($tmpid, UI_MDATA_KEY_TITLE));

        return $this->activeId;
    }


    function revert()
    {
        if(!$this->token) {
             if (UI_WARNING) $this->Base->_retMsg('No Playlist is looked by You');
            return FALSE;
        }
        $plgunid = $this->Base->gb->revertEditedPlaylist($this->token, $this->Base->sessid);
        if (PEAR::isError($plgunid)) {
            if (UI_VERBOSE) print_r($plgunid);
            if (UI_WARNING) $this->Base->_retMsg('Unable to revert to looked state');
            return FALSE;
        }
        $this->Base->_retMsg('Playlist "$1" reverted', $this->Base->_getMDataValue($this->Base->gb->_idFromGunid($plgunid), UI_MDATA_KEY_TITLE));
        $this->activeId = NULL;
        $this->token    = NULL;
        $this->Base->gb->delPref($this->Base->sessid, UI_PL_ACCESSTOKEN_KEY);

        if ($this->activate($this->Base->gb->_idFromGunid($plgunid), FALSE) !== TRUE)
            return FALSE;

        return $this->activeId;
    }

    function reportLookedPL($setMsg=FALSE)
    {
        if(is_string($saved = $this->Base->gb->loadPref($this->Base->sessid, UI_PL_ACCESSTOKEN_KEY))) {
            if ($setMsg == TRUE) $this->Base->_retMsg('Found looked Playlist');
            return TRUE;
        }
        return FALSE;
    }

    function loadLookedFromPref()
    {
        if(is_string($saved = $this->Base->gb->loadPref($this->Base->sessid, UI_PL_ACCESSTOKEN_KEY))) {
            #$this->release();
            list ($this->activeId, $this->token) = explode (':', $saved);

            $this->Base->redirUrl = UI_BROWSER.'?popup[]=_2PL.simpleManagement&popup[]=_close';
            return TRUE;
        }
        return FALSE;
    }

    function addItem($elemIds)
    {
        if (!$elemIds) {
            if (UI_WARNING) $this->Base->_retMsg('No Item(s) selected');
            return FALSE;
        }
        if (!is_array($elemIds))
            $elemIds = array($elemIds);

        foreach ($elemIds as $elemId) {
            $r = $this->Base->gb->addAudioClipToPlaylist($this->token, $elemId, $this->Base->sessid);
            if (PEAR::isError($r)) {
                if (UI_VERBOSE) print_r($r);
                $this->Base->_retMsg('Error on add item to Playlist');
                return FALSE;
            }
        }
        return TRUE;
    }

    function removeItem($elemIds)
    {
        if (!$elemIds) {
            if (UI_WARNING) $this->Base->_retMsg('No Item(s) selected');
            return FALSE;
        }
        if (!is_array($elemIds))
            $elemIds = array($elemIds);

        foreach ($elemIds as $elemId) {
            if ($this->Base->gb->delAudioClipFromPlaylist($this->token, $elemId, $this->Base->sessid) !== TRUE) {
                $this->Base->_retMsg('Cannot remove Item from Playlist');
                return FALSE;
            }
        }
        return TRUE;
    }

    function create($id)
    {
        # create PL
        # activate
        # add clip if $id is set
        if (is_array($this->activeId)) {
            $this->Base->_retMsg('Already active Playlist');
            return FALSE;
        }
        $datetime = date('Y-m-d H:i:s');
        $plid = $this->Base->gb->createPlaylist($this->Base->homeid, $datetime, $this->Base->sessid);
        if (!$plid) {
            $this->Base->_retMsg('Cannot create Playlist');
            return FALSE;
        }
        $this->Base->_setMDataValue($plid, UI_MDATA_KEY_TITLE, $datetime);
        if ($this->activate($plid)===FALSE) {
            return FALSE;
        }
        if ($id) {
            if ($this->addItem($id)!==TRUE) {
                return FALSE;
            }
        }
        #$this->redirUrl = UI_BRWOSER.'?popup=_2PL.simpleManagement';
        return $plid;
    }


    function getFlat()
    {
        $this->plwalk($this->get());
        #echo '<pre><div align="left">'; print_r($this->flat); echo '</div></pre>';
        return $this->flat;
    }


    function plwalk($arr, $parent=0, $attrs=0)
    {
        foreach ($arr['children'] as $node=>$sub) {
            if ($sub['elementname']=='playlistelement') {
                $this->plwalk($sub, $node, $sub['attrs']);
            }
            if ($sub['elementname']=='audioclip') {
                #$this->flat["$parent.$node"] = $sub['attrs'];
                #$this->flat["$parent.$node"]['type'] = $sub['elementname'];
                $this->flat[$parent] = $this->Base->_getMetaInfo($this->Base->gb->_idFromGunid($sub['attrs']['id']));
                $this->flat[$parent]['attrs'] = $attrs;
            }
            if ($sub['elementname']=='fadeinfo') {
                $this->flat[$parent]['fadein']  = GreenBox::_plTimeToSecs($sub['attrs']['fadein']);
                $this->flat[$parent]['fadeout'] = GreenBox::_plTimeToSecs($sub['attrs']['fadeout']);
                $this->flat[$parent]['fadein_ms']  = $sub['attrs']['fadein']  ? GreenBox::_plTimeToSecs($sub['attrs']['fadein'])  * 1000 : 0;
                $this->flat[$parent]['fadeout_ms'] = $sub['attrs']['fadeout'] ? GreenBox::_plTimeToSecs($sub['attrs']['fadeout']) * 1000 : 0;
            }
        }
    }


    function changeTransition($id, $type, $duration)
    {
        $curr = $this->getCurrElement($id);
        $prev = $this->getPrevElement($id);
        $next = $this->getNextElement($id);

        switch ($type) {
            case "fadeX":
                $item[$prev['attrs']['id']] =
                              array('fadeIn'  => GreenBox::_secsToPlTime($prev['fadein']),
                                    'fadeOut' => GreenBox::_secsToPlTime($duration/1000));
                $item[$id]  = array('fadeIn'  => GreenBox::_secsToPlTime($duration/1000),
                                    'fadeOut' => GreenBox::_secsToPlTime($curr['fadeout']));
            break;
            case "pause":
                $item[$prev['attrs']['id']] =
                              array('fadeIn'  => GreenBox::_secsToPlTime($prev['fadein']),
                                    'fadeOut' => GreenBox::_secsToPlTime(-$duration/1000));
                $item[$id]  = array('fadeIn'  => GreenBox::_secsToPlTime(-$duration/1000),
                                    'fadeOut' => GreenBox::_secsToPlTime($curr['fadeout']));
            break;
            case "fadeIn":
                $item[$id]  = array('fadeIn'  => GreenBox::_secsToPlTime($duration/1000),
                                    'fadeOut' => GreenBox::_secsToPlTime($curr['fadeout']));
            break;
            case "fadeOut":
                $item[$id] = array('fadeIn'  => GreenBox::_secsToPlTime($curr['fadein']),
                                   'fadeOut' => GreenBox::_secsToPlTime($duration/1000));
            break;
        }
        #print_r($item);
        foreach ($item as $i=>$val) {
            $r = $this->Base->gb->changeFadeInfo($this->token, $i, $val['fadeIn'], $val['fadeOut'], $this->Base->sessid);
            #print_r($r);
            if (PEAR::isError($r)) {
                    if (UI_VERBOSE) print_r($r);
                    $this->Base->_retMsg('ChangeFadeInfo failed');
                    return FALSE;
                }
        }
    }


    function moveItem($id, $pos)
    {
        $r = $this->Base->gb->moveAudioClipInPlaylist($this->token, $id, $pos, $this->Base->sessid);
        if (PEAR::isError($r)) {
            if (UI_VERBOSE) print_r($r);
            $this->Base->_retMsg('Cannot move item');
            return FALSE;
        }
        return TRUE;
    }


    function getCurrElement($id)
    {
        $arr = $this->getFlat();
        while ($val = current($arr)) {
            if ($val['attrs']['id'] == $id) {
                return current($arr);
            }
            next($arr);
        }
    }


    function getPrevElement($id)
    {
        $arr = $this->getFlat();
        while ($val = current($arr)) {
            if ($val['attrs']['id'] == $id) {
                return prev($arr);
            }
            next($arr);
        }
    }


    function getNextElement($id)
    {
        $arr = $this->getFlat();
        while ($val = current($arr)) {
            if ($val['attrs']['id'] == $id) {
                return next($arr);
            }
            next($arr);
        }
    }


    function changeTransitionForm($id, $type, &$mask)
    {
        $form = new HTML_QuickForm('PL_changeTransition', UI_STANDARD_FORM_METHOD, UI_HANDLER);
        $s = $this->getCurrElement($id);
        switch ($type) {
            case "fadeIn":
                $d = $this->getCurrElement($id);
                $duration = $d['fadein_ms'];
                $form->setConstants(array('headline' => '<b>'.$s['title'].'</b>'));
            break;
            case "transition":
                $d = $this->getPrevElement($id);
                $duration = $s['fadein_ms'];
                $form->setConstants(array('headline' => '<b>'.$d['title'].'</b> <-> <b>'.$s['title'].'</b>'));
            break;
            case "fadeOut":
                $d = $this->getCurrElement($id);
                $duration = $d['fadeout_ms'];
                $form->setConstants(array('headline' => '<b>'.$s['title'].'</b>'));
            break;
        }


        $form->setConstants(array('id'       => $id,
                                  'duration' => $duration)
                            );
        $this->Base->_parseArr2Form($form, $mask[$type]);
        $this->Base->_parseArr2Form($form, $mask['all']);
        $renderer =& new HTML_QuickForm_Renderer_Array(true, true);
        $form->accept($renderer);
        return $renderer->toArray();
    }


    function metaDataForm($id, $get=TRUE, $data=FALSE)
    {
        include dirname(__FILE__).'/formmask/metadata.inc.php';

        #$id = $this->activeId;

        foreach ($mask['playlist'] as $k=>$v) {
            $mask['playlist'][$k]['element']    = $this->Base->_formElementEncode($v['element']);

            ## recive data from GreenBox
            if ($get) {
                $mask['playlist'][$k]['default'] = $this->Base->_getMDataValue($id, $v['element']);
            }

            ## get data from parameter
            if (is_array($data)) {
                $mask['playlist'][$k]['default'] = $data[strtr($v['element'], '_', '.')];
            }
        }
        $form = new HTML_QuickForm('editMetaData', UI_STANDARD_FORM_METHOD, UI_HANDLER);
        $this->Base->_parseArr2Form($form, $mask['basics']);
        $this->Base->_parseArr2Form($form, $mask['playlist']);
        $this->Base->_parseArr2Form($form, $mask['buttons']);
        $form->setConstants(array('act'  => 'PL.editMetaData',
                                  'id'   => $id));
        $renderer =& new HTML_QuickForm_Renderer_Array(true, true);
        $form->accept($renderer);
        $output['main'] = $renderer->toArray();

        $form = new HTML_QuickForm('langswitch', UI_STANDARD_FORM_METHOD, UI_BROWSER);
        $this->Base->_parseArr2Form($form, $mask['langswitch']);
        $output['langswitch'] = $form->toHTML();

        #print_r($output);
        return $output;
    }


    function editMetaData(&$formdata)
    {
        include dirname(__FILE__).'/formmask/metadata.inc.php';

        #$id = $this->activeId;
        $id = $formdata['id'];

        ## first remove old entrys
        #$this->gb->replaceMetaData($id, $this->_analyzeFile($id, 'xml'), 'string', $this->sessid);

        foreach ($mask['playlist'] as $k=>$v) {
            $formdata[$this->Base->_formElementEncode($v['element'])] ? $mData[$this->Base->_formElementDecode($v['element'])] = $formdata[$this->Base->_formElementEncode($v['element'])] : NULL;
        }

        $data = $this->Base->_dateArr2Str($mData);
        foreach ($data as $key=>$val) {
            $r = $this->Base->gb->setMDataValue($id, $key, $this->Base->sessid, $val);
            if (PEAR::isError($r)) {
                $this->Base->_retMsg('Unable to set $1: $2', $key, $val);
            }
        }
        $this->Base->_retMsg('Metadata saved');
        #$this->Base->redirUrl = UI_BROWSER."?act=PL.simpleManagement&id=$id";
        $this->Base->redirUrl = UI_BROWSER."?act=PL.editMetaData&id=$id";
    }


    function deleteActive()
    {
        $id = $this->activeId;
        $this->release(FALSE);
        if ($this->Base->delete($id))
            return TRUE;
        $this->Base->_retMsg('Cannot delete this Playlist');
        return FALSE;
    }


    function _plTimeToSecs($plt, $length=4)
    {
         $arr = split(':', $plt);
         if(isset($arr[2])){ return ($arr[0]*60 + $arr[1])*60 + $arr[2]; }
         if(isset($arr[1])){ return $arr[0]*60 + $arr[1]; }
         return $arr[0];
    }

    function _secsToPlTime($s0)
    {
        $m = intval($s0 / 60);
        $r = $s0 - $m*60;
        $h = $m  / 60;
        $m = $m  % 60;
        return sprintf("%02d:%02d:%09.6f", $h, $m, $r);
    }
}
