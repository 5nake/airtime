<?php
require_once dirname(__FILE__).'/../ui_browser_init.php';

if (is_array($_REQUEST['popup'])){
    foreach ($_REQUEST['popup'] as $val) {
        switch ($val) {
            case "_reload_parent":
                $Smarty->display('popup/_reload_parent.tpl');
            break;

            case "_close":
                $Smarty->display('popup/_close.tpl');
            break;

            case "_clear_parent":
                $Smarty->display('popup/_clear_parent.tpl');
            break;

            case "login":
                $Smarty->assign('login', $uiBrowser->login($Smarty, $ui_fmask));
                $Smarty->display('popup/login.tpl');
            break;

            case "logout":
                $Smarty->assign('logouttype', 'logout');
                $Smarty->display('popup/logout.tpl');
            break;

            case "signover_1":
                $Smarty->assign('logouttype', 'signover');
                $Smarty->display('popup/logout.tpl');
            break;

            case "signover_2":
                $Smarty->assign('loginform', $uiBrowser->loginform($Smarty, $ui_fmask));
                $Smarty->display('popup/login.tpl');
            break;

            case "_2PL.simpleManagement":
                $Smarty->assign('target', 'PL.simpleManagement');
                $Smarty->display('popup/_redirector.tpl');
            break;

            case "PL.changeTransition";
                $Smarty->assign('dynform', $uiBrowser->PLAYLIST->changeTransitionForm($_REQUEST['id'], $_REQUEST['type'], $ui_fmask['PL.changeTransition']));
                $Smarty->display('popup/PL.changeTransition.tpl');
            break;
        }
    }
    die();
};

if ($uiBrowser->userid) {
  $Smarty->assign('showMenuTop', TRUE);
  $Smarty->assign('SCRATCHPAD',  $uiBrowser->SCRATCHPAD->get());
  #$Smarty->assign('PLid',        $uiBrowser->PLAYLIST->activeId);

  switch ($_REQUEST['act']){
    case "fileList":
        $Smarty->assign('structure', $uiBrowser->getStructure($uiBrowser->fid));

        $Smarty->assign('fileList', TRUE);
        if ($_REQUEST['tree']=='Y')
            $Smarty->assign('showTree', TRUE);
        else
            $Smarty->assign('showObjects', TRUE);
        $Smarty->assign('delOverride', $_REQUEST['delOverride']);
    break;

    case "permissions":
        $Smarty->assign('structure', $uiBrowser->getStructure($uiBrowser->id));

        $Smarty->assign('permissions', $uiBrowser->permissions($uiBrowser->id));
        $Smarty->assign('fileList', TRUE);
    break;


    case "uploadFileM":
        $Smarty->assign('structure', $uiBrowser->getStructure($uiBrowser->id));

        $Smarty->assign('uploadform', $uiBrowser->uploadFileM($ui_fmask['uploadFileM'], $uiBrowser->id));
    break;

    case "editItem":
        $Smarty->assign('structure', $uiBrowser->getStructure($uiBrowser->id));
        $Smarty->assign('editItem', array('type' => $uiBrowser->type=='webstream' ? 'webstream' : 'file', 'id' => $_REQUEST['id'], 'folderId' => $uiBrowser->fid));
    break;

    case "editFile":
        $Smarty->assign('structure', $uiBrowser->getStructure($uiBrowser->id));
        $Smarty->assign('editItem', array('type' => 'file', 'id' => $_REQUEST['id'], 'folderId' => $uiBrowser->fid));
    break;

    case "editWebstream":
        $Smarty->assign('structure', $uiBrowser->getStructure($uiBrowser->id));
        $Smarty->assign('editItem', array('type' => 'webstream', 'id' => $_REQUEST['id'], 'folderId' => $uiBrowser->fid));
    break;

    case "SEARCH":
        $Smarty->assign('searchForm', $uiBrowser->SEARCH->searchForm($uiBrowser->id, $ui_fmask));
        $Smarty->assign('showLibrary', TRUE);

    break;

    case "BROWSE":
        $Smarty->assign('browseForm', $uiBrowser->BROWSE->browseForm($uiBrowser->id, $ui_fmask));
        $Smarty->assign('showLibrary', TRUE);

    break;

    case "subjects":
    case "addUser":
    case "addGroup":
        $Smarty->assign('subjects', $uiBrowser->getSubjects());
        switch($_REQUEST['act']) {
            case "addUser":  $Smarty->assign('addSubjectForm', $uiBrowser->getAddSubjectForm($ui_fmask['addUser']));  break;
            case "addGroup": $Smarty->assign('addSubjectForm', $uiBrowser->getAddSubjectForm($ui_fmask['addGroup'])); break;
        }
        $Smarty->assign('showSubjects', TRUE);
    break;

    case "chgPasswd":
        $Smarty->assign('chgPasswd', $uiBrowser->chgPasswd($_REQUEST['uid'], $ui_fmask['chgPasswd']));
        $Smarty->assign('showSubjects', TRUE);
    break;

    case "groupMembers":
        $Smarty->assign('groupMembers', $uiBrowser->groupMembers($uiBrowser->id));
        $Smarty->assign('addGroupMember', $uiBrowser->addGroupMember($uiBrowser->id));
        $Smarty->assign('showSubjects', TRUE);
    break;

    case "getFile":
        $Smarty->assign('fData', $uiBrowser->getFile($uiBrowser->id));
        $Smarty->assign('showFile', TRUE);
    break;

    case "getMData":
        $Smarty->assign('fMetaData', $uiBrowser->getMdata($uiBrowser->id));
        $Smarty->assign('showFile', TRUE);
    break;


    case "_analyzeFile":
        $Smarty->assign('_analyzeFile', $uiBrowser->_analyzeFile($uiBrowser->id, 'text'));
        $Smarty->assign('showFile', TRUE);
    break;

    case "changeStationPrefs":
        $Smarty->assign('dynform', $uiBrowser->changeStationPrefs($ui_fmask['stationPrefs']));
        $Smarty->assign('changeStationPrefs', TRUE);
    break;

    case "PL.simpleManagement":
        $Smarty->assign('PL_simpleManagement', TRUE);
    break;

    case "SCHEDULER":
        $Smarty->assign('showScheduler', TRUE);
    break;
  }
}

$Smarty->display('main.tpl');
?>