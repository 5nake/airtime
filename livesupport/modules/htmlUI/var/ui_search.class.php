<?php
class uiSearch
{
    function uiSearch(&$uiBase)
    {
        $this->Base       =& $uiBase;
        $this->results    =& $_SESSION[UI_SEARCH_SESSNAME]['results'];
        $this->criteria   =& $_SESSION[UI_SEARCH_SESSNAME]['criteria'];
        $this->reloadUrl  = UI_BROWSER.'?popup[]=_reload_parent&popup[]=_close';
    }

    function setReload()
    {
        $this->Base->redirUrl = $this->reloadUrl;
    }

    function form($id, &$mask2)
    {
        include dirname(__FILE__).'/formmask/metadata.inc.php';
        $form = new HTML_QuickForm('search', UI_STANDARD_FORM_METHOD, UI_HANDLER);
        $form->setConstants(array('id'=>$id, 'counter'=>UI_SEARCH_MIN_ROWS));

        foreach ($mask['tabs']['group']['group'] as $k=>$v) {
            foreach ($mask['pages'][$v] as $val){
                $col1[$this->Base->_formElementEncode($val['element'])] = $val['label'];
                if (isset($val['relation']))
                    $col2[$this->Base->_formElementEncode($val['element'])] = $mask2['relations'][$val['relation']];
                else
                    $col2[$this->Base->_formElementEncode($val['element'])] = $mask2['relations']['standard'];
            };
        };
        for($n=1; $n<=UI_SEARCH_MAX_ROWS; $n++) {
            unset ($group);
            $form->addElement('static', 's1', NULL, "<div id='searchRow_$n'>");
            if ($n > UI_SEARCH_MIN_ROWS) $form->addElement('static', 's1_style', NULL, "<style type='text/css'>#searchRow_$n {visibility : hidden; height : 0px;}</style>");
            $sel = &$form->createElement('hierselect', "row_$n", NULL);
            $sel->setOptions(array($col1, $col2));
            $group[] = &$sel;
            $group[] = &$form->createElement('text', "row_$n".'[2]', NULL);
            $group[] = &$form->createElement('button', "dropRow_$n", 'Drop', array('onClick' => "SearchForm_dropRow('$n')"));
            $form->addGroup($group);
            $form->addElement('static', 's2', NULL, "</div id='searchRow_$n'>");
        }
        $this->Base->_parseArr2Form($form, $mask2['search']);
        $form->setConstants($this->criteria['form']);
        $form->validate();
        $renderer =& new HTML_QuickForm_Renderer_Array(true, true);
        $form->accept($renderer);
        $output['dynform'] = $renderer->toArray();
        #print_r($output);
        return $output;
    }


    function newsearch(&$formdata)
    {
        $this->results                  = NULL;
        $this->criteria['conditions']   = NULL;
        $this->criteria['offset']       = NULL;

        $this->criteria['operator']         = $formdata['operator'];
        $this->criteria['filetype']         = $formdata['filetype'];
        $this->criteria['limit']            = $formdata['limit'];
        $this->criteria['form']['operator'] = $formdata['operator'];    ## $criteria['form'] is used for retransfer to form ##
        $this->criteria['form']['filetype'] = $formdata['filetype'];
        $this->criteria['form']['limit']    = $formdata['limit'];

        foreach ($formdata as $key=>$val) {
            if (is_array($val) && strlen($val[2])) {
                $this->criteria['conditions'][$key] = array('cat' => $this->Base->_formElementDecode($val[0]),
                                                            'op'  => $val[1],
                                                            'val' => stripslashes($val[2]));
                $this->criteria['form'][$key]       = array(0     => $this->Base->_formElementDecode($val[0]),
                                                            1     => $val[1],
                                                            2     => stripslashes($val[2]));
            }
        }
        $this->Base->redirUrl = UI_BROWSER.'?act=SEARCH';
        $this->searchDB();
    }

    function searchDB()
    {
        $this->results = NULL;
        #print_r($this->criteria);
        $results = $this->Base->gb->localSearch($this->criteria, $this->Base->sessid);
        foreach ($results['results'] as $rec) {
            $this->results['items'][] = $this->Base->_getMetaInfo($this->Base->gb->_idFromGunid($rec));
        }
        $this->pagination($results);
    }


    function pagination(&$results)
    {
        $this->results['count'] = $results['cnt'];
        $this->results['next']  = $results['cnt'] > $this->criteria['offset'] + $this->criteria['limit'] ? TRUE : FALSE;
        $this->results['prev']  = $this->criteria['offset'] > 0 ? TRUE : FALSE;

        $p = 1;
        for ($n = 1; $n <= ceil($results['cnt'] / $this->criteria['limit']); $n = $n+$p) {
            $p = bcpow(10, floor($n/10)); echo "$p<br>";
            $this->results['pages'][$n-1] = $n;
        }

        array_pop($this->results['pages']);
        $this->results['pages'][ceil($results['cnt'] / $this->criteria['limit'])-1] = '>>';
    }


    function reOrder($by)
    {
        $this->criteria['offset'] = NULL;

        if ($this->criteria['orderby'] == $by && !$this->criteria['desc'])
            $this->criteria['desc'] = TRUE;
        else
            $this->criteria['desc'] = FALSE;

        $this->criteria['orderby'] = $by;
        $this->setReload();
        $this->searchDB();
    }


    function clear()
    {
        #$this->results    = NULL;
        $this->criteria['form'] = NULL;
        $this->setReload();
    }

    function setOffset($page)
    {
        $o =& $this->criteria['offset'];
        $l =& $this->criteria['limit'];

        if ($page == 'next') {
            $o += $l;
        } elseif ($page == 'prev') {
            $o -= $l;
        } elseif (is_numeric($page)) {
            $o = $l * $page;
        }
        $this->setReload();
        $this->searchDB();
    }
}
?>
