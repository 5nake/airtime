<?php
class uiScheduler extends uiCalendar
{
    function uiScheduler(&$uiBase)
    {
        $this->curr   =& $_SESSION[UI_CALENDAR_SESSNAME]['current'];
        if (!is_array($this->curr)) {
            $this->curr['view']     = 'month';
            $this->curr['year']     = date("Y");
            $this->curr['month']    = date("m");
            $this->curr['day']      = date('d');
            $this->curr['hour']     = date('H');
        }

        $this->Base =& $uiBase;
        $this->reloadUrl = UI_BROWSER.'?popup[]=_reload_parent&popup[]=_close';

        $this->uiCalendar();
        $this->initXmlRpc();
    }


    function setReload()
    {
        $this->Base->redirUrl = $this->reloadUrl;
    }

    function set($arr)
    {
        extract($arr);

        if (isset($view))  $this->curr['view'] = $view;
        if (isset($year))  $this->curr['year'] = $year;
        if (isset($day))   $this->curr['day']  = sprintf('%02d', $day);
        if (isset($hour))  $this->curr['hour'] = sprintf('%02d', $hour);
        if (is_numeric($month))
                    $this->curr['month'] = sprintf('%02d', $month);

        $stampNow    = $this->_datetime2timestamp($this->curr['year'].$this->curr['month'].$this->curr['day']);
        $stampTarget = $stampNow;
        if ($month=='++')
            $stampTarget = strtotime("+1 month", $stampNow);
        if ($month=='--')
            $stampTarget = strtotime("-1 month", $stampNow);
        if ($week=='++')
            $stampTarget = strtotime("+1 week", $stampNow);
        if ($week=='--')
            $stampTarget = strtotime("-1 week", $stampNow);
        $this->curr['year']     = strftime("%Y", $stampTarget);
        $this->curr['month']    = strftime("%m", $stampTarget);
        $this->curr['day']      = strftime("%d", $stampTarget);
    }


    function _datetime2timestamp($i)
    {
        $formatted = $i[0].$i[1].$i[2].$i[3].'-'.$i[4].$i[5].'-'.$i[6].$i[7].strrchr($i, 'T');
        #echo "iiiii: $i ffff:".$formatted;
        return strtotime($formatted);
    }


    function getDayUsage($year, $month, $day)
    {
        $date = $year.$month.$day;
        $arr = $this->displayScheduleMethod($date.'T00:00:00', $date.'T23:59:59.999999');
        if (!count($arr))
            return FALSE;
        #print_r($arr);
        return $arr;
    }

    function getDayUsagePercentage($year, $month, $day)
        {
        if (!$arr = $this->getDayUsage($year, $month, $day))
            return false;

        foreach ($arr as $val) {
            $duration += ($this->_datetime2timestamp($val['end'])-$this->_datetime2timestamp($val['start']))/86400*100;
        }
        return $duration;
    }


    function getDayTiming($year, $month, $day)
    {
        if (!$arr = $this->getDayUsage($year, $month, $day))
            return false;

        ## !! bug in strtotime. zeigt 8h sp�ter an als reines datum, wenn Txx:xx:xx verwendet wird !!

        $day_start = $this->_datetime2timestamp($year.$month.$day.'T00:00:00');
        $day_end   = $this->_datetime2timestamp($year.$month.$day.'T23:59:59');

        $curr = current($arr);
        if (strtotime($curr['start']) > $day_start)                     ## insert gap if first entry start after 00:00:00
            $list[] = array(
                        'type'      => 'firstgap',
                        'pos'       => 0,
                        'length'    => strtotime($curr['start']) - $day_start -1
                      );

        while ($curr = current($arr)) {
            $list[] = array(
                        'type'      => 'entry',
                        'pos'       => strtotime($curr['start']) - $day_start,
                        'length'    => strtotime($curr['end']) - strtotime($curr['start']),
                        'entry'     => $curr
                      );

            if ($next = next($arr)) {
                if ($next['start'] > $curr['end']+1)  ## insert gap between entrys
                    $list[] = array(
                                'type'      => 'gap',
                                'pos'       => strtotime($curr['start'])-$day_start,
                                'length'    => strtotime($next['start']) - strtotime($curr['end']) -1,
                              );
            }
            else {
                if (strtotime($curr['end']) < $day_end)        ## insert gap if prev entry was not until midnight
                    $list[] = array(
                                'type'      => 'lastgap',
                                'pos'       => strtotime($curr['end']) - $day_start,
                                'length'    => $day_end-strtotime($curr['end']),
                              );
            }

        }
        #print_r($list);
        return $list;
    }


    function _scheduledDays($period)
    {
        if ($period=='month') {
            require_once 'Calendar/Month/Weekdays.php';
            $Period = new Calendar_Month_Weekdays($this->curr['year'], $this->curr['month'], $this->firstDayOfWeek);
            $Period->build();
        } elseif ($period=='week') {
            require_once 'Calendar/Week.php';
            $Period = new Calendar_Week ($this->curr['year'], $this->curr['month'], $this->curr['day'], $this->firstDayOfWeek);
            $Period->build();
        } else {
            return array();
        }
        $d = $Period->fetch();
        $corrMonth = $d->thisMonth()<=12 ? sprintf('%02d', $d->thisMonth()) : '01';   ## due to bug in
        $corrYear  = $d->thisMonth()<=12 ? $d->thisYear() : $d->thisYear()+1;         ## Calendar_Month_Weekdays
        $first = array('day'   => sprintf('%02d', $d->thisDay()),
                       'month' => $corrMonth,
                       'year'  => $corrYear
                 );

        while ($l = $Period->fetch()) {
            $d = $l;
        }
        $corrMonth = $d->thisMonth()<=12 ? sprintf('%02d', $d->thisMonth()) : '01';   ## due to bug in
        $corrYear  = $d->thisMonth()<=12 ? $d->thisYear() : $d->thisYear()+1;         ## Calendar_Month_Weekdays
        $last = array('day'   => sprintf('%02d', $d->thisDay()),
                      'month' => $corrMonth,
                      'year'  => $corrYear
                );


        #echo "F:".$first['year'].$first['month'].$first['day']." L:".$last['year'].$last['month'].$last['day'];
        $days = $this->_reciveScheduledDays($first['year'].$first['month'].$first['day'], $last['year'].$last['month'].$last['day']);
        foreach ($days as $val) {
            $selections[] = new Calendar_Day($val['year'], $val['month'], $val['day']);
        }
        return $selections;
    }


    function _reciveScheduledDays($dfrom, $dto)
    {
        $dfrom = $dfrom.'T00:00:00';
        $dto   = $dto.'T23:59:59';
        if (($pArr  = $this->displayScheduleMethod($dfrom, $dto)) === FALSE)
            return array(FALSE);;

        foreach ($pArr as $val) {
            #print_r($val);
            $pStampArr[] = array('start' => $this->_datetime2timestamp($val['start']),
                                 'end'   => $this->_datetime2timestamp($val['end']));
        }
        if (is_array($pStampArr)) {
            #print_r($pStampArr);
            for ($n=$this->_datetime2timestamp($dfrom); $n<=$this->_datetime2timestamp($dto); $n+=86400) {
                foreach ($pStampArr as $val) {
                    if ($val['start'] < $n+86400 && $val['end'] >= $n) {
                        $days[date('Ymd', $n)] = array('year'  => date('Y', $n),
                                                                       'month' => date('m', $n),
                                                                       'day'   => date('d', $n));
                    }
                }
            }
            return $days;
        }
        return array(FALSE);
    }


    function copyPlFromSP()
    {
        foreach ($this->Base->SCRATCHPAD->get() as $val) {
            if (strtolower($val['type'])=='playlist' && $val['id']!=$this->Base->PLAYLIST->activeId)
                $this->playlists[] = $val;
        }
    }

    function _isError($r)
    {
        if (is_array($r['error'])) {
            $this->Base->_retMsg('Error: $1', str_replace("\n", "\\n", addslashes($r['error']['message'])));
            return TRUE;
        }
        return FALSE;
    }

    ## XML-RPC wrapper methods ############################################################################################
    function initXmlRpc()
    {
        include_once dirname(__FILE__).'/SchedulerPhpClient.class.php';
        $this->spc =& SchedulerPhpClient::factory($this->Base->dbc, $mdefs, $this->Base->config);
    }


    function uploadPlaylistMethod(&$formdata)
    {
        $gunid = $formdata['gunid'];
        $datetime = $this->curr['year'].$this->curr['month'].$this->curr['day'].'T'.$formdata['time'];
        #echo "Schedule Gunid: $gunid  At: ".$datetime;
        $r = $this->spc->UploadPlaylistMethod($this->Base->sessid, $gunid, $datetime.UI_TIMEZONE);
        #print_r($r);
        if ($this->_isError($r))
            return FALSE;
        if (isset($r['scheduleEntryId']))
            $this->Base->_retMsg('ScheduleId: $1', $r['scheduleEntryId']);
    }


    function displayScheduleMethod($from, $to)
    {
        #echo $from.$to;
        $r = $this->spc->displayScheduleMethod($this->Base->sessid, $from, $to);
        if ($this->_isError($r))
            return FALSE;
        return $r;
    }
}
?>
